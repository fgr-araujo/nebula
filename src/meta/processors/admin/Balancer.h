/* Copyright (c) 2019 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#ifndef META_ADMIN_BALANCERR_H_
#define META_ADMIN_BALANCERR_H_

#include <gtest/gtest_prod.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include "kvstore/KVStore.h"
#include "network/NetworkUtils.h"
#include "time/WallClock.h"
#include "meta/processors/admin/AdminClient.h"
#include "meta/processors/admin/BalanceTask.h"
#include "meta/processors/admin/BalancePlan.h"

namespace nebula {
namespace meta {
/**
There are two interfaces public:
 * Balance:  it will construct a balance plan and invoked it. If last balance plan is not succeeded, it will
 * try to resume it.
 *
 * Rollback: In many cases, if some plan failed forever, we call this interface to rollback.

Some notes:
1. Balance will generate balance plan according to current active hosts and parts allocation
2. For the plan, we hope after moving the least parts , it will reach a reasonable state.
3. Only one balance plan could be invoked at the same time.
4. Each balance plan has one id, and we could show the status by "balance id" command
   and after FO, we could resume the balance plan by type "balance" again.
5. Each balance plan contains many balance tasks, the task represents the minimum movement unit.
6. We save the whole balancePlan state in kvstore to do failover.
7. Each balance task contains serval steps. And it should be executed step by step.
8. One task failed will result in the whole balance plan failed.
9. Currently, we hope tasks for the same part could be invoked serially
 * */
class Balancer {
    FRIEND_TEST(BalanceTest, BalancePartsTest);
    FRIEND_TEST(BalanceTest, NormalTest);
    FRIEND_TEST(BalanceTest, RecoveryTest);

public:
    static Balancer* instance(kvstore::KVStore* kv) {
        static std::unique_ptr<AdminClient> client(new AdminClient());
        static std::unique_ptr<Balancer> balancer(new Balancer(kv, std::move(client)));
        return balancer.get();
    }

    ~Balancer() = default;

    /*
     * Return Error if reject the balance request, otherwise return balance id.
     * */
    StatusOr<BalanceID> balance();

    /**
     * TODO(heng): Rollback some specific balance id
     */
    Status rollback(BalanceID id) {
        return Status::Error("unplemented, %ld", id);
    }

    /**
     * TODO(heng): Only generate balance plan for our users.
     * */
    const BalancePlan* preview() {
        return plan_.get();
    }

    /**
     * TODO(heng): Execute balance plan from outside.
     * */
    Status execute(BalancePlan plan) {
        UNUSED(plan);
        return Status::Error("Unsupport it yet!");
    }

    /**
     * TODO(heng): Execute specific balance plan by id.
     * */
    Status execute(BalanceID id) {
        UNUSED(id);
        return Status::Error("Unsupport it yet!");
    }

private:
    Balancer(kvstore::KVStore* kv, std::unique_ptr<AdminClient> client)
        : kv_(kv)
        , client_(std::move(client)) {
        executor_.reset(new folly::CPUThreadPoolExecutor(1));
    }
    /*
     * When the balancer failover, we should recovery the status.
     * */
    bool recovery();

    /**
     * Build balance plan and save it in kvstore.
     * */
    Status buildBalancePlan();

    std::vector<BalanceTask> genTasks(GraphSpaceID spaceId);

    void getHostParts(GraphSpaceID spaceId,
                      std::unordered_map<HostAddr, std::vector<PartitionID>>& hostParts,
                      int32_t& totalParts);

    void calDiff(const std::unordered_map<HostAddr, std::vector<PartitionID>>& hostParts,
                 const std::vector<HostAddr>& activeHosts,
                 std::vector<HostAddr>& newlyAdded,
                 std::vector<HostAddr>& lost);

    StatusOr<HostAddr> hostWithMinimalParts(
                        const std::unordered_map<HostAddr, std::vector<PartitionID>>& hostParts,
                        PartitionID partId);

    void balanceParts(BalanceID balanceId,
                      GraphSpaceID spaceId,
                      std::unordered_map<HostAddr, std::vector<PartitionID>>& newHostParts,
                      int32_t totalParts,
                      std::vector<BalanceTask>& tasks);


    std::vector<std::pair<HostAddr, int32_t>>
    sortedHostsByParts(const std::unordered_map<HostAddr, std::vector<PartitionID>>& hostParts);

private:
    std::atomic_bool  running_{false};
    kvstore::KVStore* kv_ = nullptr;
    std::unique_ptr<AdminClient> client_{nullptr};
    // Current running plan.
    std::unique_ptr<BalancePlan> plan_{nullptr};
    std::unique_ptr<folly::Executor> executor_;
};

}  // namespace meta
}  // namespace nebula

#endif  // META_ADMIN_BALANCERR_H_
