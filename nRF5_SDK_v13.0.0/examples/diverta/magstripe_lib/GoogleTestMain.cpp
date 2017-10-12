/*
 * GoogleTestMain.cpp
 *
 *  Created on: 2014/11/10
 *      Author: kazu
 */

#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
