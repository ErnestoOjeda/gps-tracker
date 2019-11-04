/*************************************************************************
 * Main Stable Version of Wander Firmware --- Particle Boron Tailored (M4)
 *************************************************************************
 *
 * Features:
 *  - Logistic that is able to know when to do a push of data
 *  - Refresh Actual Location
 *  - Sends dataObject to a firebase, curently:
 *    ** battery_life
 *    ** sat_fix
 *    ** latitude
 *    ** longitude
 *    ** altitude
 *    ** course
 *    ** speed
 *    ** # of satellites lock
 *
 * Particle Functions
 *  - actual-location => Just refresh and send an updated dataObject to firebase
 *  - debug-device [true,false] => Can be used to call a device into debug mode
 *
 * @version 0.6
 */
float wander_os_version = 0.6;

/**************************** CONFIGURE BELOW ****************************/
/*************************************************************************/

/**
 * Enable/Disable for debugging purposes. Uses Particle Console.
 *
 * You can also perform a particle API call for debug-device with a true parameter And
 * it will enable the debug-mode, meaning you'll see whats happening on a code level.
 */
    uint8_t DEBUG = false;

/**
 * How many minutes will need to pass before each dataObject push to firebase
 *
 * If debug is enabled it will automatically go to 1 min push
 * 
 * For this to work, you'll need to ad ==> || sat_fix == true && millis() > start_timer + (60000*push)
 * on the main loop
 */
    const uint8_t data_push = 15;

/************************* VARIABLES & LIBRARIES *************************/
/*************************************************************************/

/**
 * LIBRARIES
 */
    #include <TinyGPS++.h>

/**
 * LIBRARY CALLS
 */
    FuelGauge   fuel;
    TinyGPSPlus gps;

/**
 * SETTINGS & MEMORY
 */

    // Device
    bool refresh_location = false;
    int  battery_life;

    // Minutes before dataObject Push
    uint8_t push = data_push;

    // GPS Satellites Lock
    bool sat_fix = false;

    // GPS Data
    float start_timer = 0;

    double latitude;
    double longitude;
    double altitude;
    double course;
    double speed;
    double satellites;

/****************************** SETUP & LOOP *****************************/
/*************************************************************************/

void setup() {
    // Wait for hardware serial to appear
    while (!Serial1);

    /********************* Enable the GPS Module *************************/
    Serial1.begin(9600);

    /********************* Establishing the Timer ************************/
    start_timer = millis();

    /********************* We Subscribe to Actual Location Function API Calls ************************/
    Particle.function("actual-location", get_actual_location);

    /********************* We Subscribe to Debug Function API Calls ************************/
    Particle.function("debug-device", debug_device);

    /***************************** DEBUG *********************************/
    if( DEBUG ) {
        Particle.publish("DEBUG", "Initializing Particle ID: " + System.deviceID(), PRIVATE);
        Particle.publish("DEBUG", "Wander OS Version: " + String(wander_os_version), PRIVATE);

        // Changing push to debug Module
        push = 1;
    }
}

/*************************************************************************/

void loop() {

    // We use the sat_fix variable to keep looping every 60 sec until we get the gps data => [sat_fix == false && millis() > start_timer + (60000)]
    // Or if we ask for a refresh of the location... => [refresh_location == true]
    if( sat_fix == false && millis() > start_timer + (60000) || refresh_location == true ) {

        /********************** Resetting the Timer & Refresh Location *************************/
        start_timer = millis();

        /************************* Battery Life *****************************/
        battery_life = fuel.getSoC();
        battery_life = map(battery_life, 0, 80, 0, 100);

        /************************* GET GPS DATA *****************************/
        gps_variables();

        /********************* Send GPS to Firebase *************************/
        if( sat_fix == true ) {
            // Send the event to Particle Cloud
            particlecloud_push( battery_life, "true", latitude, longitude, altitude, course, speed, satellites );
            
            // Because we now have sent a valid location, we reset the request
            refresh_location = false;
        }
        else {
            // Only Used for debug purposes currently
            if( DEBUG ) {
                particlecloud_push( battery_life, "false", 0, 0, 0, 0, 0, 0 );
            }
        }
    }

}

/******************************** Functions ******************************/
/*************************************************************************/

int get_actual_location(String command) {
    refresh_location = true;

    /***************************** DEBUG *********************************/
    if( DEBUG ) {
      Particle.publish("DEBUG", "Getting Actual Location...", PRIVATE);
    }
}

/*************************************************************************/

int debug_device(String command) {
    if(command == "true") {
        DEBUG = true;

        // Changing push to debug Mode
        push = 1;

        Particle.publish("DEBUG", "DEBUG mode enabled", PRIVATE);
    }
    else {
        DEBUG = false;

        // Changing push to standard Mode
        push = data_push;

        Particle.publish("DEBUG", "DEBUG mode disabled", PRIVATE);
    }
}

/*************************************************************************/

void gps_variables() {

    // For the boron to be able to access the data it will need to get it
    // Using the serial channel and add it to the TinyGPSPlus library to manage it
    while( Serial1.available() > 0 ) {
        gps.encode(Serial1.read());
    }

    // To be able to establish the variables, we need to get a read first
    if( gps.location.isValid() ) {

        /**
         * Variables available to send to database
         *    LATITUDE: gps.location.lat() | In degrees (double)
         *    LONGITUDE: gps.location.lng() | In degrees (double)
         *    ALTITUDE: gps.altitude.feet() or .meters() or .miles() or .kilometers() | (double)
         *    COURSE: gps.course.deg() | (double)
         *    SPEED: gps.speed.mph() or .knots() or .mps() or .kmph()  | (double)
         *    SATS: gps.satellites.value()
         */

        latitude   = gps.location.lat();
        longitude  = gps.location.lng();
        altitude   = gps.altitude.feet();
        course     = gps.course.deg();
        speed      = gps.speed.mph();
        satellites = gps.satellites.value();

        // We tell the device that we are fix to a sat and have the data
        sat_fix = true;

        /***************************** DEBUG *********************************/
        if( DEBUG ) {
            Particle.publish("DEBUG", "GPS SAT FIX", PRIVATE);
        }
    }
    else {
        // We tell the device that we are not fix to a sat and there is no data
        sat_fix = false;

        /***************************** DEBUG *********************************/
        if( DEBUG ) {
            Particle.publish("DEBUG", "**NO GPS SAT FIX", PRIVATE);
        }
    }
}

/***************************** DO NOT EDIT BELOW **************************/
/*************************************************************************/

void particlecloud_push( double battery_life, char* sat_fix, double latitude, double longitude, double altitude,
                         double course, double speed, double satellites ) {
    Particle.publish("wander-hardware-push", "{\"batteryLife\":\"" +  String(battery_life) + "\", \"satFix\": \"" + String(sat_fix) + "\", \"gpsData\": { \"latitude\": \"" + String(latitude) +
                                                                                                                          "\", \"longitude\": \"" + String(longitude) +
                                                                                                                          "\", \"altitude\": \"" + String(altitude) +
                                                                                                                          "\", \"course\": \"" + String(course) +
                                                                                                                          "\", \"speed\": \"" + String(speed) +
                                                                                                                          "\", \"satellites\": \"" + String(satellites) + "\" } }", PRIVATE);
}
