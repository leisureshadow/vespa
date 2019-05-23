// Copyright 2019 Oath Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/searchcore/proton/matching/handlerecorder.h>
#include <vespa/vespalib/gtest/gtest.h>

#include <vespa/log/log.h>

LOG_SETUP("handle_recorder_test");

using search::fef::MatchDataDetails;
using search::fef::TermFieldHandle;
using namespace proton::matching;

using HandleSet = HandleRecorder::HandleSet;

void
register_normal_handle(TermFieldHandle handle)
{
    HandleRecorder::register_handle(handle, MatchDataDetails::Normal);
}

void
register_cheap_handle(TermFieldHandle handle)
{
    HandleRecorder::register_handle(handle, MatchDataDetails::Cheap);
}

TEST(HandleRecorderTest, can_record_both_normal_and_cheap_handles)
{
    HandleRecorder recorder;
    {
        HandleRecorder::Binder binder(recorder);
        register_normal_handle(3);
        register_cheap_handle(5);
        register_normal_handle(7);
    }
    EXPECT_EQ(HandleSet({3, 7}), recorder.get_normal_handles());
    EXPECT_EQ(HandleSet({5}), recorder.get_cheap_handles());
    EXPECT_EQ("normal: [3,7], cheap: [5]", recorder.to_string());
}

TEST(HandleRecorderTest, the_same_handle_can_be_in_both_normal_and_cheap_set)
{
    HandleRecorder recorder;
    {
        HandleRecorder::Binder binder(recorder);
        register_normal_handle(3);
        register_cheap_handle(3);
    }
    EXPECT_EQ(HandleSet({3}), recorder.get_normal_handles());
    EXPECT_EQ(HandleSet({3}), recorder.get_cheap_handles());
}

GTEST_MAIN_RUN_ALL_TESTS()