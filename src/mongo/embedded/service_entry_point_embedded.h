/**
 *    Copyright (C) 2018-present MongoDB, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the Server Side Public License, version 1,
 *    as published by MongoDB, Inc.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    Server Side Public License for more details.
 *
 *    You should have received a copy of the Server Side Public License
 *    along with this program. If not, see
 *    <http://www.mongodb.com/licensing/server-side-public-license>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the Server Side Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#pragma once

#include <cstddef>
#include <memory>

#include "mongo/base/status.h"
#include "mongo/bson/bsonobjbuilder.h"
#include "mongo/db/dbmessage.h"
#include "mongo/logv2/log_severity.h"
#include "mongo/rpc/message.h"
#include "mongo/transport/service_entry_point.h"
#include "mongo/transport/session.h"
#include "mongo/util/duration.h"
#include "mongo/util/future.h"

namespace mongo {

/**
 * The entry point into mongod. Just a wrapper around assembleResponse.
 */
class ServiceEntryPointEmbedded final : public ServiceEntryPoint {
    ServiceEntryPointEmbedded(const ServiceEntryPointEmbedded&) = delete;
    ServiceEntryPointEmbedded& operator=(const ServiceEntryPointEmbedded&) = delete;

public:
    ServiceEntryPointEmbedded() = default;
    Future<DbResponse> handleRequest(OperationContext* opCtx,
                                     const Message& request) noexcept override;

    void startSession(std::shared_ptr<transport::Session> session) override;
    void endAllSessions(transport::Session::TagMask tags) override;
    Status start() override;
    bool shutdown(Milliseconds timeout) override;
    void appendStats(BSONObjBuilder* bob) const override;
    size_t numOpenSessions() const override;
    logv2::LogSeverity slowSessionWorkflowLogSeverity() override;

private:
    class Hooks;
};

}  // namespace mongo
