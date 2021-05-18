/**
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr

estimating the value of Pi using Monte Carlo method
implemented by referring to
https://www.geeksforgeeks.org/estimating-value-pi-using-monte-carlo/
 */

#include "sdmt.h"

#include <iostream>
#include <random>

#define INTERVAL 1000

int main() {
    using namespace std;
    // init sdmt manage
    // if there is archive, recover it
    SDMT::init("./config_cpp_test.xml", true);

    double* pi;
    int* circle_points, *square_points;

    // check this execution is first try
    // if it is, create segments
    // else get recoverd value
    if (!SDMT::exist("mc_pi")) {
        SDMT::register_segment("mc_pi", SDMT_DOUBLE, SDMT_SCALAR, {});
        SDMT::register_segment("mc_circle", SDMT_INT, SDMT_SCALAR, {});
        SDMT::register_segment("mc_square", SDMT_INT, SDMT_SCALAR, {});

        pi = SDMT::doubleptr("mc_pi");
        circle_points = SDMT::intptr("mc_circle");
        square_points = SDMT::intptr("mc_square");

        *circle_points = 0;
        *square_points = 0;
    } else {
        pi = SDMT::doubleptr("mc_pi");
        circle_points = SDMT::intptr("mc_circle");
        square_points = SDMT::intptr("mc_square");
    }

    // get current iteration sequence
    int& it = SDMT::iter();

    // start sdmt module
    SDMT::start();

    // init rand()
    srand(time(nullptr));

    // total random numbers generated = possible x
    // value * possible y values
    for ( ; it < (INTERVAL * INTERVAL); it++) {
        // randomly generated x and y values
        double rand_x = double(rand() % (INTERVAL + 1)) / INTERVAL;
        double rand_y = double(rand() % (INTERVAL + 1)) / INTERVAL;

        // distance between (x, y) from the origin
        double origin_dist = rand_x * rand_x + rand_y * rand_y;

        // checking if (x, y) lies inside the define
        // circle with R=1
        if (origin_dist <= 1)
            (*circle_points)++;

        // total number of points generated
        (*square_points)++;

        // estimated pi after this iteration
        (*pi) = double(4 * (*circle_points)) / (*square_points);

        // checkpoint for every 10 interval
        if (it % (INTERVAL*10) == 0) {
            SDMT::checkpoint(1);
            std::cout << it << "th iterations has processed,"
                << "current Pi is " << (*pi) << std::endl;
        }
    }

    // final result
    cout << "final estimation of Pi = " << (*pi) << std::endl; 

    // finalize sdmt module
    SDMT::finalize();
  
    return 0; 

}
