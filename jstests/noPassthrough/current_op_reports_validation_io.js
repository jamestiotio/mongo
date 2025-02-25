/**
 * Verifies that the 'dataThroughputLastSecond' and 'dataThroughputAverage' fields appear in the
 * currentOp output while running validation.
 *
 * @tags: [requires_fsync, requires_wiredtiger, requires_persistence]
 */
import {configureFailPoint} from "jstests/libs/fail_point_util.js";

const dbName = "test";
const collName = "currentOpValidation";

const conn = MongoRunner.runMongod();

let db = conn.getDB(dbName);
let coll = db.getCollection(collName);

coll.drop();

assert.commandWorked(coll.createIndex({a: 1}));
for (let i = 0; i < 5; i++) {
    assert.commandWorked(coll.insert({a: i}));
}

// The throttle is off by default.
assert.commandWorked(db.adminCommand({setParameter: 1, maxValidateMBperSec: 1}));

// Simulate each record being 512KB.
const cursorDataSizeFailPoint = configureFailPoint(db, "fixedCursorDataSizeOf512KBForDataThrottle");

// This fail point comes after we've traversed the record store, so currentOp should have some
// validation statistics once we hit this fail point.
const pauseFailPoint = configureFailPoint(db, "pauseCollectionValidationWithLock");

// Forces a checkpoint to make the background validation see the data.
assert.commandWorked(db.adminCommand({fsync: 1}));

TestData.dbName = dbName;
TestData.collName = collName;
const awaitValidation = startParallelShell(() => {
    assert.commandWorked(
        db.getSiblingDB(TestData.dbName).getCollection(TestData.collName).validate({
            background: true
        }));
}, conn.port);

pauseFailPoint.wait();

const curOpFilter = {
    'command.validate': collName
};

let curOp = assert.commandWorked(db.currentOp(curOpFilter));
assert(curOp.inprog.length == 1);
assert(curOp.inprog[0].hasOwnProperty("dataThroughputLastSecond") &&
       curOp.inprog[0].hasOwnProperty("dataThroughputAverage"));

curOp = conn.getDB("admin").aggregate([{$currentOp: {}}, {$match: curOpFilter}]).toArray();
assert(curOp.length == 1);
assert(curOp[0].hasOwnProperty("dataThroughputLastSecond") &&
       curOp[0].hasOwnProperty("dataThroughputAverage"));

// Finish up validating the collection.
cursorDataSizeFailPoint.off();
pauseFailPoint.off();

// Setting this to 0 turns off the throttle.
assert.commandWorked(db.adminCommand({setParameter: 1, maxValidateMBperSec: 0}));

awaitValidation();

MongoRunner.stopMongod(conn);