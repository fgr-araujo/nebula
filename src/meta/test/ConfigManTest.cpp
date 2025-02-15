/* Copyright (c) 2018 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include "base/Base.h"
#include <gtest/gtest.h>
#include <folly/String.h>
#include "fs/TempDir.h"
#include "meta/test/TestUtils.h"
#include "meta/GflagsManager.h"
#include "meta/ClientBasedGflagsManager.h"
#include "meta/KVBasedGflagsManager.h"
#include "meta/processors/configMan/GetConfigProcessor.h"
#include "meta/processors/configMan/SetConfigProcessor.h"
#include "meta/processors/configMan/ListConfigsProcessor.h"
#include "meta/processors/configMan/RegConfigProcessor.h"

DECLARE_int32(load_data_interval_secs);
DECLARE_int32(load_config_interval_secs);

// some gflags to register
DEFINE_int64(int64_key_immutable, 100, "test");
DEFINE_int64(int64_key, 101, "test");
DEFINE_bool(bool_key, false, "test");
DEFINE_double(double_key, 1.23, "test");
DEFINE_string(string_key, "something", "test");
DEFINE_string(test0, "v0", "test");
DEFINE_string(test1, "v1", "test");
DEFINE_string(test2, "v2", "test");
DEFINE_string(test3, "v3", "test");
DEFINE_string(test4, "v4", "test");

namespace nebula {
namespace meta {

TEST(ConfigManTest, ConfigProcessorTest) {
    fs::TempDir rootPath("/tmp/ConfigProcessorTest.XXXXXX");
    std::unique_ptr<kvstore::KVStore> kv(TestUtils::initKV(rootPath.path()));

    cpp2::ConfigItem item1;
    item1.set_module(cpp2::ConfigModule::STORAGE);
    item1.set_name("k1");
    item1.set_type(cpp2::ConfigType::STRING);
    item1.set_mode(cpp2::ConfigMode::MUTABLE);
    item1.set_value("v1");

    cpp2::ConfigItem item2;
    item2.set_module(cpp2::ConfigModule::STORAGE);
    item2.set_name("k2");
    item2.set_type(cpp2::ConfigType::STRING);
    item2.set_value("v2");

    // set and get without register
    {
        cpp2::SetConfigReq req;
        req.set_item(item1);

        auto* processor = SetConfigProcessor::instance(kv.get());
        auto f = processor->getFuture();
        processor->process(req);
        auto resp = std::move(f).get();
        ASSERT_NE(cpp2::ErrorCode::SUCCEEDED, resp.get_code());
    }
    {
        cpp2::ConfigItem item;
        item.set_module(cpp2::ConfigModule::STORAGE);
        item.set_name("k1");
        cpp2::GetConfigReq req;
        req.set_item(item);

        auto* processor = GetConfigProcessor::instance(kv.get());
        auto f = processor->getFuture();
        processor->process(req);
        auto resp = std::move(f).get();
        ASSERT_NE(cpp2::ErrorCode::SUCCEEDED, resp.get_code());
    }
    // register config item1 and item2
    {
        std::vector<cpp2::ConfigItem> items;
        items.emplace_back(item1);
        items.emplace_back(item2);
        cpp2::RegConfigReq req;
        req.set_items(items);

        auto* processor = RegConfigProcessor::instance(kv.get());
        auto f = processor->getFuture();
        processor->process(req);
        auto resp = std::move(f).get();
        ASSERT_EQ(cpp2::ErrorCode::SUCCEEDED, resp.get_code());
    }
    // set and get string config item1
    {
        cpp2::SetConfigReq req;
        req.set_item(item1);

        auto* processor = SetConfigProcessor::instance(kv.get());
        auto f = processor->getFuture();
        processor->process(req);
        auto resp = std::move(f).get();
        ASSERT_EQ(cpp2::ErrorCode::SUCCEEDED, resp.get_code());
    }
    {
        cpp2::ConfigItem item;
        item.set_module(cpp2::ConfigModule::STORAGE);
        item.set_name("k1");
        cpp2::GetConfigReq req;
        req.set_item(item);

        auto* processor = GetConfigProcessor::instance(kv.get());
        auto f = processor->getFuture();
        processor->process(req);
        auto resp = std::move(f).get();
        ASSERT_EQ(cpp2::ErrorCode::SUCCEEDED, resp.get_code());
        ASSERT_EQ(item1, resp.get_items().front());
    }
    // get config not existed
    {
        cpp2::ConfigItem item;
        item.set_module(cpp2::ConfigModule::STORAGE);
        item.set_name("not_existed");
        cpp2::GetConfigReq req;
        req.set_item(item);

        auto* processor = GetConfigProcessor::instance(kv.get());
        auto f = processor->getFuture();
        processor->process(req);
        auto resp = std::move(f).get();
        ASSERT_EQ(0, resp.get_items().size());
    }
    // list all configs in a module
    {
        cpp2::ListConfigsReq req;
        req.set_module(cpp2::ConfigModule::STORAGE);

        auto* processor = ListConfigsProcessor::instance(kv.get());
        auto f = processor->getFuture();
        processor->process(req);
        auto resp = std::move(f).get();

        ASSERT_EQ(cpp2::ErrorCode::SUCCEEDED, resp.get_code());
        ASSERT_EQ(2, resp.get_items().size());
        auto ret1 = resp.get_items().front();
        auto ret2 = resp.get_items().back();
        if (ret1.get_name() == "k1") {
            ASSERT_EQ(ret1, item1);
            ASSERT_EQ(ret2, item2);
        } else {
            ASSERT_EQ(ret1, item2);
            ASSERT_EQ(ret2, item1);
        }
    }

    // register another config in other module, list all configs in all module
    cpp2::ConfigItem item3;
    item3.set_module(cpp2::ConfigModule::META);
    item3.set_name("k1");
    item3.set_type(cpp2::ConfigType::STRING);
    item3.set_value("v1");

    {
        std::vector<cpp2::ConfigItem> items;
        items.emplace_back(item3);
        cpp2::RegConfigReq req;
        req.set_items(items);

        auto* processor = RegConfigProcessor::instance(kv.get());
        auto f = processor->getFuture();
        processor->process(req);
        auto resp = std::move(f).get();
        ASSERT_EQ(cpp2::ErrorCode::SUCCEEDED, resp.get_code());
    }
    {
        cpp2::ListConfigsReq req;
        req.set_module(cpp2::ConfigModule::ALL);

        auto* processor = ListConfigsProcessor::instance(kv.get());
        auto f = processor->getFuture();
        processor->process(req);
        auto resp = std::move(f).get();

        ASSERT_EQ(cpp2::ErrorCode::SUCCEEDED, resp.get_code());
        ASSERT_EQ(3, resp.get_items().size());
    }
}

ConfigItem toConfigItem(const cpp2::ConfigItem& item) {
    VariantType value;
    switch (item.get_type()) {
        case cpp2::ConfigType::INT64:
            value = *reinterpret_cast<const int64_t*>(item.get_value().data());
            break;
        case cpp2::ConfigType::BOOL:
            value = *reinterpret_cast<const bool*>(item.get_value().data());
            break;
        case cpp2::ConfigType::DOUBLE:
            value = *reinterpret_cast<const double*>(item.get_value().data());
            break;
        case cpp2::ConfigType::STRING:
            value = item.get_value();
            break;
    }
    return ConfigItem(item.get_module(), item.get_name(), item.get_type(), item.get_mode(), value);
}

TEST(ConfigManTest, MetaConfigManTest) {
    FLAGS_load_data_interval_secs = 1;
    FLAGS_load_config_interval_secs = 1;
    fs::TempDir rootPath("/tmp/MetaConfigManTest.XXXXXX");
    uint32_t localMetaPort = 0;
    auto sc = TestUtils::mockMetaServer(localMetaPort, rootPath.path());
    TestUtils::createSomeHosts(sc->kvStore_.get());

    auto threadPool = std::make_shared<folly::IOThreadPoolExecutor>(1);
    IPv4 localIp;
    network::NetworkUtils::ipv4ToInt("127.0.0.1", localIp);

    auto module = cpp2::ConfigModule::STORAGE;
    auto client = std::make_shared<MetaClient>(threadPool,
        std::vector<HostAddr>{HostAddr(localIp, sc->port_)});
    client->waitForMetadReady();
    client->setGflagsModule(module);

    ClientBasedGflagsManager cfgMan(client.get());
    cfgMan.module_ = module;
    // mock some test gflags to meta
    {
        auto mode = meta::cpp2::ConfigMode::MUTABLE;
        cfgMan.gflagsDeclared_.emplace_back(toThriftConfigItem(
            module, "int64_key_immutable", cpp2::ConfigType::INT64, cpp2::ConfigMode::IMMUTABLE,
            toThriftValueStr(cpp2::ConfigType::INT64, 100L)));
        cfgMan.gflagsDeclared_.emplace_back(toThriftConfigItem(
            module, "int64_key", cpp2::ConfigType::INT64,
            mode, toThriftValueStr(cpp2::ConfigType::INT64, 101L)));
        cfgMan.gflagsDeclared_.emplace_back(toThriftConfigItem(
            module, "bool_key", cpp2::ConfigType::BOOL,
            mode, toThriftValueStr(cpp2::ConfigType::BOOL, false)));
        cfgMan.gflagsDeclared_.emplace_back(toThriftConfigItem(
            module, "double_key", cpp2::ConfigType::DOUBLE,
            mode, toThriftValueStr(cpp2::ConfigType::DOUBLE, 1.23)));
        std::string defaultValue = "something";
        cfgMan.gflagsDeclared_.emplace_back(toThriftConfigItem(
            module, "string_key", cpp2::ConfigType::STRING,
            mode, toThriftValueStr(cpp2::ConfigType::STRING, defaultValue)));
        cfgMan.registerGflags();
    }

    // try to set/get config not registered
    {
        std::string name = "not_existed";
        auto type = cpp2::ConfigType::INT64;

        sleep(FLAGS_load_config_interval_secs + 1);
        // get/set without register
        auto setRet = cfgMan.setConfig(module, name, type, 101l).get();
        ASSERT_FALSE(setRet.ok());
        auto getRet = cfgMan.getConfig(module, name).get();
        ASSERT_FALSE(getRet.ok());
    }
    // immutable configs
    {
        std::string name = "int64_key_immutable";
        auto type = cpp2::ConfigType::INT64;

        // register config as immutable and try to update
        auto setRet = cfgMan.setConfig(module, name, type, 101l).get();
        ASSERT_FALSE(setRet.ok());

        // get immutable config
        auto getRet = cfgMan.getConfig(module, name).get();
        ASSERT_TRUE(getRet.ok());
        auto item = toConfigItem(getRet.value().front());
        auto value = boost::get<int64_t>(item.value_);
        ASSERT_EQ(value, 100);

        sleep(FLAGS_load_config_interval_secs + 1);
        ASSERT_EQ(FLAGS_int64_key_immutable, 100);
    }
    // mutable config
    {
        std::string name = "int64_key";
        auto type = cpp2::ConfigType::INT64;
        ASSERT_EQ(FLAGS_int64_key, 101);

        // update config
        auto setRet = cfgMan.setConfig(module, name, type, 102l).get();
        ASSERT_TRUE(setRet.ok());

        // get from meta server
        auto getRet = cfgMan.getConfig(module, name).get();
        ASSERT_TRUE(getRet.ok());
        auto item = toConfigItem(getRet.value().front());
        auto value = boost::get<int64_t>(item.value_);
        ASSERT_EQ(value, 102);

        // get from cache
        sleep(FLAGS_load_config_interval_secs + 1);
        ASSERT_EQ(FLAGS_int64_key, 102);
    }
    {
        std::string name = "bool_key";
        auto type = cpp2::ConfigType::BOOL;
        ASSERT_EQ(FLAGS_bool_key, false);

        // update config
        auto setRet = cfgMan.setConfig(module, name, type, true).get();
        ASSERT_TRUE(setRet.ok());

        // get from meta server
        auto getRet = cfgMan.getConfig(module, name).get();
        ASSERT_TRUE(getRet.ok());
        auto item = toConfigItem(getRet.value().front());
        auto value = boost::get<bool>(item.value_);
        ASSERT_EQ(value, true);

        // get from cache
        sleep(FLAGS_load_config_interval_secs + 1);
        ASSERT_EQ(FLAGS_bool_key, true);
    }
    {
        std::string name = "double_key";
        auto type = cpp2::ConfigType::DOUBLE;
        ASSERT_EQ(FLAGS_double_key, 1.23);

        // update config
        auto setRet = cfgMan.setConfig(module, name, type, 3.14).get();
        ASSERT_TRUE(setRet.ok());

        // get from meta server
        auto getRet = cfgMan.getConfig(module, name).get();
        ASSERT_TRUE(getRet.ok());
        auto item = toConfigItem(getRet.value().front());
        auto value = boost::get<double>(item.value_);
        ASSERT_EQ(value, 3.14);

        // get from cache
        sleep(FLAGS_load_config_interval_secs + 1);
        ASSERT_EQ(FLAGS_double_key, 3.14);
    }
    {
        std::string name = "string_key";
        auto type = cpp2::ConfigType::STRING;
        ASSERT_EQ(FLAGS_string_key, "something");

        // update config
        std::string newValue = "abc";
        auto setRet = cfgMan.setConfig(module, name, type, newValue).get();
        ASSERT_TRUE(setRet.ok());

        // get from meta server
        auto getRet = cfgMan.getConfig(module, name).get();
        ASSERT_TRUE(getRet.ok());
        auto item = toConfigItem(getRet.value().front());
        auto value = boost::get<std::string>(item.value_);
        ASSERT_EQ(value, "abc");

        // get from cache
        sleep(FLAGS_load_config_interval_secs + 1);
        ASSERT_EQ(FLAGS_string_key, "abc");
    }
    {
        auto ret = cfgMan.listConfigs(module).get();
        ASSERT_TRUE(ret.ok());
        ASSERT_EQ(ret.value().size(), 5);
    }
}

TEST(ConfigManTest, MockConfigTest) {
    FLAGS_load_config_interval_secs = 1;
    fs::TempDir rootPath("/tmp/MockConfigTest.XXXXXX");
    uint32_t localMetaPort = 0;
    auto sc = TestUtils::mockMetaServer(localMetaPort, rootPath.path());

    // mock one ClientBaseGflagsManager, and do some update value in console, check if it works
    auto threadPool = std::make_shared<folly::IOThreadPoolExecutor>(1);
    IPv4 localIp;
    network::NetworkUtils::ipv4ToInt("127.0.0.1", localIp);
    auto module = cpp2::ConfigModule::STORAGE;
    auto type = cpp2::ConfigType::STRING;
    auto mode = cpp2::ConfigMode::MUTABLE;

    auto client = std::make_shared<MetaClient>(threadPool,
        std::vector<HostAddr>{HostAddr(localIp, sc->port_)});
    client->waitForMetadReady();
    client->setGflagsModule(module);
    ClientBasedGflagsManager clientCfgMan(client.get());
    clientCfgMan.module_ = module;

    for (int i = 0; i < 5; i++) {
        std::string name = "test" + std::to_string(i);
        std::string value = "v" + std::to_string(i);
        clientCfgMan.gflagsDeclared_.emplace_back(
                toThriftConfigItem(module, name, type, mode, value));
    }
    clientCfgMan.registerGflags();

    auto consoleClient = std::make_shared<MetaClient>(threadPool,
        std::vector<HostAddr>{HostAddr(localIp, sc->port_)});
    ClientBasedGflagsManager console(consoleClient.get());
    // update in console
    for (int i = 0; i < 5; i++) {
        std::string name = "test" + std::to_string(i);
        std::string value = "updated" + std::to_string(i);
        auto setRet = console.setConfig(module, name, type, value).get();
        ASSERT_TRUE(setRet.ok());
    }
    // get in console
    for (int i = 0; i < 5; i++) {
        std::string name = "test" + std::to_string(i);
        std::string value = "updated" + std::to_string(i);

        auto getRet = console.getConfig(module, name).get();
        ASSERT_TRUE(getRet.ok());
        auto item = toConfigItem(getRet.value().front());
        ASSERT_EQ(boost::get<std::string>(item.value_), value);
    }

    // check values in ClientBaseGflagsManager
    sleep(FLAGS_load_config_interval_secs + 1);
    ASSERT_EQ(FLAGS_test0, "updated0");
    ASSERT_EQ(FLAGS_test1, "updated1");
    ASSERT_EQ(FLAGS_test2, "updated2");
    ASSERT_EQ(FLAGS_test3, "updated3");
    ASSERT_EQ(FLAGS_test4, "updated4");
}

}  // namespace meta
}  // namespace nebula

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    folly::init(&argc, &argv, true);
    google::SetStderrLogging(google::INFO);
    return RUN_ALL_TESTS();
}

