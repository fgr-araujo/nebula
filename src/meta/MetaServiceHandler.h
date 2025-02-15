/* Copyright (c) 2018 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#ifndef META_METASERVICEHANDLER_H_
#define META_METASERVICEHANDLER_H_

#include "base/Base.h"
#include <mutex>
#include "interface/gen-cpp2/MetaService.h"
#include "kvstore/KVStore.h"

namespace nebula {
namespace meta {

class MetaServiceHandler final : public cpp2::MetaServiceSvIf {
public:
    explicit MetaServiceHandler(kvstore::KVStore* kv)
                : kvstore_(kv) {}

    /**
     * Parts distribution related operations.
     * */
    folly::Future<cpp2::ExecResp>
    future_createSpace(const cpp2::CreateSpaceReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_dropSpace(const cpp2::DropSpaceReq& req) override;

    folly::Future<cpp2::ListSpacesResp>
    future_listSpaces(const cpp2::ListSpacesReq& req) override;

    folly::Future<cpp2::GetSpaceResp>
    future_getSpace(const cpp2::GetSpaceReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_addHosts(const cpp2::AddHostsReq& req) override;

    folly::Future<cpp2::ListHostsResp>
    future_listHosts(const cpp2::ListHostsReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_removeHosts(const cpp2::RemoveHostsReq& req) override;

    folly::Future<cpp2::GetPartsAllocResp>
    future_getPartsAlloc(const cpp2::GetPartsAllocReq& req) override;

    /**
     * Custom kv related operations.
     * */
    folly::Future<cpp2::ExecResp>
    future_multiPut(const cpp2::MultiPutReq& req) override;

    folly::Future<cpp2::GetResp>
    future_get(const cpp2::GetReq& req) override;

    folly::Future<cpp2::MultiGetResp>
    future_multiGet(const cpp2::MultiGetReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_remove(const cpp2::RemoveReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_removeRange(const cpp2::RemoveRangeReq& req) override;

    folly::Future<cpp2::ScanResp>
    future_scan(const cpp2::ScanReq& req) override;

    /**
     * Schema related operations.
     * */
    folly::Future<cpp2::ExecResp>
    future_createTag(const cpp2::CreateTagReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_alterTag(const cpp2::AlterTagReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_dropTag(const cpp2::DropTagReq& req) override;

    folly::Future<cpp2::GetTagResp>
    future_getTag(const cpp2::GetTagReq &req) override;

    folly::Future<cpp2::ListTagsResp>
    future_listTags(const cpp2::ListTagsReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_createEdge(const cpp2::CreateEdgeReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_alterEdge(const cpp2::AlterEdgeReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_dropEdge(const cpp2::DropEdgeReq& req) override;

    folly::Future<cpp2::GetEdgeResp>
    future_getEdge(const cpp2::GetEdgeReq& req) override;

    folly::Future<cpp2::ListEdgesResp>
    future_listEdges(const cpp2::ListEdgesReq& req) override;

    /**
     * User manager
     **/
    folly::Future<cpp2::ExecResp>
    future_createUser(const cpp2::CreateUserReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_dropUser(const cpp2::DropUserReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_alterUser(const cpp2::AlterUserReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_grantRole(const cpp2::GrantRoleReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_revokeRole(const cpp2::RevokeRoleReq& req) override;

    folly::Future<cpp2::GetUserResp>
    future_getUser(const cpp2::GetUserReq& req) override;

    folly::Future<cpp2::ListUsersResp>
    future_listUsers(const cpp2::ListUsersReq& req) override;

    folly::Future<cpp2::ListRolesResp>
    future_listRoles(const cpp2::ListRolesReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_changePassword(const cpp2::ChangePasswordReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_checkPassword(const cpp2::CheckPasswordReq& req) override;

    /**
     * HeartBeat
     * */
    folly::Future<cpp2::HBResp>
    future_heartBeat(const cpp2::HBReq& req) override;

    folly::Future<cpp2::BalanceResp>
    future_balance(const cpp2::BalanceReq& req) override;

    folly::Future<cpp2::ExecResp>
    future_regConfig(const cpp2::RegConfigReq &req) override;

    folly::Future<cpp2::GetConfigResp>
    future_getConfig(const cpp2::GetConfigReq &req) override;

    folly::Future<cpp2::ExecResp>
    future_setConfig(const cpp2::SetConfigReq &req) override;

    folly::Future<cpp2::ListConfigsResp>
    future_listConfigs(const cpp2::ListConfigsReq &req) override;

private:
    kvstore::KVStore* kvstore_ = nullptr;
};

}  // namespace meta
}  // namespace nebula
#endif  // META_METASERVICEHANDLER_H_
