#include "z_probe.h"
#if defined(PROBE_PIN) && (PROBE_PIN > -1)
#include "Marlin.h"
#include "stepper.h"
#include "temperature.h"

float Probe_Bed(float x_pos, float y_pos, int n)
{
    //returns Probed Z average height
    float ProbeDepth[n];
    float ProbeDepthAvg=0;
    
    //force bed heater off for probing
    int save_bed_targ = target_raw_bed;
    target_raw_bed = 0;
    WRITE(HEATER_BED_PIN,LOW);
    
    if (Z_HOME_DIR==-1)
    {
      //int probe_flag =1;
      float meas = 0;
      int fails = 0;
      saved_feedrate = feedrate;
      saved_feedmultiply = feedmultiply;
      feedmultiply = 100;
      //previous_millis_cmd = millis();
      
      //Move to probe position
      if (x_pos >= 0) destination[X_AXIS]=x_pos;
      if (y_pos >= 0) destination[Y_AXIS]=y_pos;
      //destination[Z_AXIS]=current_position[Z_AXIS];
      destination[Z_AXIS]=Z_HOME_RETRACT_MM;
      feedrate = 9000;
      prepare_move();

	  enable_endstops(true);
      SERIAL_ECHO("PRE-PROBE current_position[Z_AXIS]=");SERIAL_ECHOLN(current_position[Z_AXIS]);

	  SERIAL_ECHOLN("Ready to probe...");

        //Probe bed n times
        //*******************************************************************************************Bed Loop*************************************
        for(int8_t i=0; i < n ; i++)
        {
            //int z = 0;

			//fast probe 
			//plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]); 
			destination[Z_AXIS] = 1.1 * Z_MAX_LENGTH * Z_HOME_DIR; 
			feedrate = homing_feedrate[Z_AXIS]; 
			plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder); 
			st_synchronize();
		
			//feedrate = 0.0;
            
            SERIAL_ECHO("current_position[Z_AXIS]=");SERIAL_ECHOLN(current_position[Z_AXIS]);
            if(endstop_z_hit == true)
            {
	            SERIAL_ECHO("endstops_trigsteps[Z_AXIS]=");SERIAL_ECHOLN(endstops_trigsteps[Z_AXIS]);
	            ProbeDepth[i]= endstops_trigsteps[Z_AXIS] / axis_steps_per_unit[Z_AXIS];
	            meas = ProbeDepth[i];
	            SERIAL_ECHO("ProbeDepth[");SERIAL_ECHO(i);SERIAL_ECHO("]=");SERIAL_ECHOLN(ProbeDepth[i]);
            	//*************************************************************************************************************
		        if (i > 0 ) //Second probe has happened so compare results
		        {
		            if (abs(ProbeDepth[i] - ProbeDepth[i - 1]) > .05)
		            { //keep going until readings match to avoid sticky bed
		              SERIAL_ECHO("Probing again: ");
		              SERIAL_ECHO(ProbeDepth[i]); SERIAL_ECHO(" - "); SERIAL_ECHO(ProbeDepth[i - 1]);SERIAL_ECHO(" = "); SERIAL_ECHOLN(abs(ProbeDepth[i] - ProbeDepth[i - 1]));
		              meas = ProbeDepth[i];
		              i--; i--; //Throw out both that don't match because we don't know which one is accurate
		              if(fails++ > 4) break;
		            }
		        }
	        }else{
	        	SERIAL_ECHOLN("Probe not triggered.");
	        	i=n-1;
	        }
            //**************************************************************************************************************************************************
            //fast move clear
		    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], meas, current_position[E_AXIS]);
		    destination[Z_AXIS] = Z_HOME_RETRACT_MM;
		    feedrate = fast_home_feedrate[Z_AXIS];
		    plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
		    st_synchronize();

            //check z stop isn't still triggered
            if ( READ(X_MIN_PIN) != X_ENDSTOPS_INVERTING )
            {
                SERIAL_ECHOLN("Poking Stuck Bed:");
                destination[Z_AXIS] = -1; prepare_move();
                destination[Z_AXIS] = Z_HOME_RETRACT_MM; prepare_move();
			    st_synchronize();
                i--; //Throw out this meaningless measurement
            }
            feedrate = 0;
        } //end probe loop
		#ifdef ENDSTOPS_ONLY_FOR_HOMING
		  enable_endstops(false);
		#endif
		 
		feedrate = saved_feedrate;
		feedmultiply = saved_feedmultiply;
		//previous_millis_cmd = millis();
		endstops_hit_on_purpose();
    }
    for(int8_t i=0;i<n;i++)
    {
    	ProbeDepthAvg += ProbeDepth[i];
    }
    ProbeDepthAvg /= n;
    SERIAL_ECHO("Probed Z="); SERIAL_ECHOLN(ProbeDepthAvg);
    SERIAL_ECHO("RAW current_position[Z_AXIS]=");SERIAL_ECHOLN(current_position[Z_AXIS]);
    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], Z_HOME_RETRACT_MM, current_position[E_AXIS]);
    current_position[Z_AXIS] = Z_HOME_RETRACT_MM;

    target_raw_bed = save_bed_targ;
    return ProbeDepthAvg;
 }

void probe_init()
{
    SET_INPUT(PROBE_PIN);
    WRITE(PROBE_PIN,HIGH);
}

/*Crash1 - G29 to Probe and stop on Bed
G29 will probe bed at least twice at 3 points and take an average. G30 will probe bed at it's current location.
Z stop should be set slightly below bed height. Solder stub wire to each hole in huxley bed and attach a ring terminal under spring.
Wire bed probe to A2 on Melzi and duplicate cap/resistor circuit in schematic.
  
Use something like this in the start.gcode file:
G29 		;Probe bed for Z height
G92 Z0		;Set Z to Probed Depth
G1 Z5 F200	;Lift Z out of way
*/
void probe_3points()
{
    float Probe_Avg, Point1, Point2, Point3;
    Point1 = Probe_Bed(15,15,PROBE_N);
    Point2 = Probe_Bed(X_MAX_LENGTH - 20,15,PROBE_N) ;
    Point3 = Probe_Bed(X_MAX_LENGTH/2,Y_MAX_LENGTH - 5,PROBE_N);
    Probe_Avg = (Point1 + Point2 + Point3) / 3;
    //destination[2] = Probe_Avg;
    //feedrate = homing_feedrate[Z_AXIS];
    //prepare_move();
    SERIAL_ECHOLN("**************************************");       
    SERIAL_ECHO("Point1 ="); SERIAL_ECHOLN(Point1);
    SERIAL_ECHO("Point2 ="); SERIAL_ECHOLN(Point2);
    SERIAL_ECHO("Point3 ="); SERIAL_ECHOLN(Point3);
    SERIAL_ECHO("Probed Average="); SERIAL_ECHOLN(Probe_Avg);
    SERIAL_ECHOLN("**************************************");       
}

void probe_1point()
{
    float  Point;
    Point = Probe_Bed(-1,-1,PROBE_N);
    //destination[2] = Point +1;
    //feedrate = homing_feedrate[Z_AXIS];
    //prepare_move();
    SERIAL_ECHOLN("**************************************");       
    SERIAL_ECHO("Probed Z="); SERIAL_ECHOLN(Point);
}

void probe_status()
{
    SERIAL_ECHO("Probe Status = "); SERIAL_ECHOLN(READ(PROBE_PIN));
}

#endif //defined(PROBE_PIN) > -1


