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

#include <boost/preprocessor/control/iif.hpp>
#include <utility>

#include <boost/move/utility_core.hpp>
#include <boost/optional/optional.hpp>

#include "mongo/s/catalog_cache_loader.h"
#include "mongo/util/assert_util_core.h"
#include "mongo/util/decorable.h"

namespace mongo {
namespace {

const auto catalogCacheLoaderDecoration =
    ServiceContext::declareDecoration<std::unique_ptr<CatalogCacheLoader>>();

}  // namespace

CatalogCacheLoader::CollectionAndChangedChunks::CollectionAndChangedChunks() = default;

CatalogCacheLoader::CollectionAndChangedChunks::CollectionAndChangedChunks(
    OID epoch,
    Timestamp timestamp,
    UUID collUuid,
    bool unsplittable,
    const BSONObj& collShardKeyPattern,
    const BSONObj& collDefaultCollation,
    bool collShardKeyIsUnique,
    boost::optional<TypeCollectionTimeseriesFields> collTimeseriesFields,
    boost::optional<TypeCollectionReshardingFields> collReshardingFields,
    bool allowMigrations,
    std::vector<ChunkType> chunks)
    : epoch(std::move(epoch)),
      timestamp(std::move(timestamp)),
      uuid(std::move(collUuid)),
      unsplittable(unsplittable),
      shardKeyPattern(collShardKeyPattern),
      defaultCollation(collDefaultCollation),
      shardKeyIsUnique(collShardKeyIsUnique),
      timeseriesFields(std::move(collTimeseriesFields)),
      reshardingFields(std::move(collReshardingFields)),
      allowMigrations(allowMigrations),
      changedChunks(std::move(chunks)) {}

void CatalogCacheLoader::set(ServiceContext* serviceContext,
                             std::unique_ptr<CatalogCacheLoader> loader) {
    auto& catalogCacheLoader = catalogCacheLoaderDecoration(serviceContext);
    invariant(!catalogCacheLoader);

    catalogCacheLoader = std::move(loader);
}

void CatalogCacheLoader::clearForTests(ServiceContext* serviceContext) {
    auto& catalogCacheLoader = catalogCacheLoaderDecoration(serviceContext);
    invariant(catalogCacheLoader);

    catalogCacheLoader.reset();
}

CatalogCacheLoader& CatalogCacheLoader::get(ServiceContext* serviceContext) {
    auto& catalogCacheLoader = catalogCacheLoaderDecoration(serviceContext);
    invariant(catalogCacheLoader);

    return *catalogCacheLoader;
}

CatalogCacheLoader& CatalogCacheLoader::get(OperationContext* opCtx) {
    return get(opCtx->getServiceContext());
}

}  // namespace mongo
