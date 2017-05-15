//
// Created by Sam Royston on 1/16/17.
//

#ifndef CONTROL_TEST_SUITE_H
#define CONTROL_TEST_SUITE_H

#include <random>
#include "cxxtest/cxxtest/TestSuite.h"
#include "src/bot.h"
#include "src/state.h"
#include "src/utils.h"
#include "src/network_manager.h"
#include "src/agent.h"
#include <unistd.h>


#endif //CONTROL_TEST_SUITE_H

class Tests : public CxxTest::TestSuite{
public:
    void test_bot(){

    }
    void test_state(){

    }
    void test_utils(){

    }
    void test_network(){
        Bot *bot = new Bot("asdasdsa");
        StateAction s;
        s.x = 2;
        s.y = 4;
        int j = 25;
        StateAction d = bot->train_action(s);
        int i;
        for(i=0; i < j; i++){
            usleep(100000);
            s.x = rand() % 10;
            s.y = rand() % 10;
            bot->train_action(s);
        }
        TS_ASSERT_EQUALS(i, j);

    }
    void test_agent(){
        srand (static_cast <unsigned> (time(0)));
        StateActionSpace s(100, 100, 4);
        for (int i = 0; i < 1000; ++i) {
            int x = rand() % s.count_x;
            int y = rand() % s.count_y;
            int a = rand() % s.action_count;
            float v = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            s.set_action_value(x, y, a, v);
        }
        s.save("test_Q_data");

        StateActionSpace load_s("test_Q_data");

        TS_ASSERT_EQUALS(s.size(), load_s.size());
        for (int i = 0; i < s.size() && i < load_s.size(); ++i) {
            TS_ASSERT_EQUALS(s.at((unsigned long) i).size(), load_s.at((unsigned long) i).size());
            for (int j = 0; j < s.at((unsigned long) i).size(); ++j) {
                StateAction s0 = s.at((unsigned long) i).at((unsigned long) j);
                StateAction s1 = load_s.at((unsigned long) i).at((unsigned long) j);
                TS_ASSERT_EQUALS(s0.value, s1.value);
                TS_ASSERT_EQUALS(s0.x, s1.x);
                TS_ASSERT_EQUALS(s0.y, s1.y);
                TS_ASSERT_EQUALS(s0.visits, s1.visits);
                TS_ASSERT_EQUALS(s0.action, s1.action);
            }
        }

        system("rm test_Q_data");
    }
};