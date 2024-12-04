#!/bin/bash
gcc controller.c -o control -lrt -pthread
gcc temperature.c -o temp -lrt -pthread
gcc air_humidity.c -o air -lrt -pthread
gcc soil_moisture.c -o soil -lrt -pthread
gcc sunlight.c -o sun -lrt -pthread
gcc shutdown.c -o shutdown -lrt -pthread