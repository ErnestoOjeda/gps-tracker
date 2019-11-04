/*************************************************************************
 * Main Stable Version of Wander Firmware - Particle E-series Tailored (M3)
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
 * @version 0.7
 */
float wander_os_version = 0.7;

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

    /**
     * PROCCESS GPS DATA
     *
     * For the E-series to be able to access the data it will need to get it
     * Using the serial channel and add it to the TinyGPSPlus library to manage it
     * We break the loop once we get the gps data...meanwhile is going to keep looking for satellites
     */
    while (Serial1.available() > 0) {
        if (gps.encode(Serial1.read())) {
            if ( gps.location.isValid() ) {
                break;
            }
            else {
                Particle.publish("Debug", "Searching for satellites...", PRIVATE);
                sat_fix = false;
                delay(10000);
            }
        }
    }

    /**
     * SEND DATA TO CLOUD
     *
     * We limit the time of every push to the first one, once the device is started
     * Or if we ask for a refresh of the location... => [refresh_location == true]
     */
    if( sat_fix == false && millis() > start_timer + (60000) || refresh_location == true ) {

        /********************** Resetting the Timer *************************/
        start_timer = millis();

        if( DEBUG ) {
            Particle.publish("DEBUG", "Preparing the data...", PRIVATE);
        }

        /************************* Battery Life *****************************/
        battery_life = fuel.getSoC();
        battery_life = map(battery_life, 0, 80, 0, 100);

        /************************* Get the GPS data *****************************/
        if ( gps.location.isValid() ) {
            latitude   = gps.location.lat(); // In degrees (double)
            longitude  = gps.location.lng(); // In degrees (double)
            altitude   = gps.altitude.feet(); // or .meters() or .miles() or .kilometers() | (double)
            course     = gps.course.deg(); // (double)
            speed      = gps.speed.mph(); // or .knots() or .mps() or .kmph()  | (double)
            satellites = gps.satellites.value();

            // We indicate that we have the gps data
            sat_fix = true;
        }

        /********************* Send data to the cloud *************************/
        if( sat_fix == true ) {
            // Send the event to Particle Cloud
            particle_cloud_push( battery_life, "true", latitude, longitude, altitude, course, speed, satellites );
            
            // Because we now have sent a valid location, we reset the request
            refresh_location = false;
        }
        else {
            // Only Used for debug purposes currently
            if( DEBUG ) {
                particle_cloud_push( battery_life, "false", 0, 0, 0, 0, 0, 0 );
            }
        }
    }
}

/*************************************************************************/
/**************************** DO NOT EDIT BELOW **************************/
/*************************************************************************/

int get_actual_location(String command) {
    refresh_location = true;

    /***************************** DEBUG *********************************/
    if( DEBUG ) {
      Particle.publish("DEBUG", "Requesting Actual Location...", PRIVATE);
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

void particle_cloud_push( double battery_life, char* sat_fix, double latitude, double longitude, double altitude,
                         double course, double speed, double satellites ) {
    Particle.publish("wander-hardware-push", "{\"batteryLife\":\"" +  String(battery_life) + "\", \"satFix\": \"" + String(sat_fix) + "\", \"gpsData\": { \"latitude\": \"" + String(latitude) +
                                                                                                                          "\", \"longitude\": \"" + String(longitude) +
                                                                                                                          "\", \"altitude\": \"" + String(altitude) +
                                                                                                                          "\", \"course\": \"" + String(course) +
                                                                                                                          "\", \"speed\": \"" + String(speed) +
                                                                                                                          "\", \"satellites\": \"" + String(satellites) + "\" } }", PRIVATE);
}
