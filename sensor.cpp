/*
 * Author: Jon Trulson <jtrulson@ics.com>
 * Copyright (c) 2015 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <unistd.h>
#include <iostream>
#include "l298.h"

using namespace std;

int main ()
{
  //! [Interesting]

  // Instantiate one of the 2 possible DC motors on a L298 Dual
  // H-Bridge.  For controlling a stepper motor, see the l298-stepper
  // example.

	upm::L298* motor2 = new upm::L298(5, 4, 9);
	upm::L298* motor1 = new upm::L298(3, 6, 8);



  cout << "Starting motor at 50% for 3 seconds..." << endl;
  motor1->setSpeed(65);
  motor2->setSpeed(60);
  motor1->setDirection(upm::L298::DIR_CW);
  motor2->setDirection(upm::L298::DIR_CW);
  motor1->enable(true);
  motor2->enable(true);


  usleep(1000*2000);

  cout << "Reversing direction..." << endl;
  motor1->setDirection(upm::L298::DIR_NONE); // fast stop
  motor2->setDirection(upm::L298::DIR_NONE); // fast stop
  motor1->setDirection(upm::L298::DIR_CCW);
  motor2->setDirection(upm::L298::DIR_CCW);
  motor1->enable(true);
  motor2->enable(true);

  usleep(1000*1000);




  motor1->setSpeed(0);
  motor2->setSpeed(0);

  motor1->enable(false);
  motor2->enable(false);
 //! [Interesting]

  cout << "Exiting..." << endl;


  delete motor1;
  delete motor2;
  return 0;
}
