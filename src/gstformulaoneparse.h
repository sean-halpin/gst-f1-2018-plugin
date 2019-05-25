#ifndef __GST_FORMULAONEPARSE_H__
#define __GST_FORMULAONEPARSE_H__

#include <gst/gst.h>

G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GST_TYPE_FORMULAONEPARSE \
  (gst_formula_one_parse_get_type())
#define GST_FORMULAONEPARSE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_FORMULAONEPARSE, GstFormulaOneParse))
#define GST_FORMULAONEPARSE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_FORMULAONEPARSE, GstFormulaOneParseClass))
#define GST_IS_FORMULAONEPARSE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_FORMULAONEPARSE))
#define GST_IS_FORMULAONEPARSE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_FORMULAONEPARSE))

typedef struct _GstFormulaOneParse GstFormulaOneParse;
typedef struct _GstFormulaOneParseClass GstFormulaOneParseClass;

struct _GstFormulaOneParse
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  gboolean silent;
};

struct _GstFormulaOneParseClass
{
  GstElementClass parent_class;
};

GType gst_formula_one_parse_get_type(void);

struct PacketHeader
{
  u_int16_t m_packetFormat;    // 2018
  u_int8_t m_packetVersion;    // Version of this packet type, all start from 1
  u_int8_t m_packetId;         // Identifier for the packet type, see below
  u_int64_t m_sessionUID;      // Unique identifier for the session
  float m_sessionTime;         // Session timestamp
  u_int32_t m_frameIdentifier; // Identifier for the frame the data was retrieved on
  u_int8_t m_playerCarIndex;   // Index of player's car in the array
} typedef PacketHeader;

// MOTION PACKET
// The motion packet gives physics data for all the cars being driven. There is additional data for the car being driven with the goal of being able to drive a motion platform setup.
// N.B. For the normalised vectors below, to convert to float values divide by 32767.0f. 16-bit signed values are used to pack the data and on the assumption that direction values are always between -1.0f and 1.0f.
// Frequency: Rate as specified in menus
// Size: 1341 bytes

struct CarMotionData
{
  float m_worldPositionX;       // World space X position
  float m_worldPositionY;       // World space Y position
  float m_worldPositionZ;       // World space Z position
  float m_worldVelocityX;       // Velocity in world space X
  float m_worldVelocityY;       // Velocity in world space Y
  float m_worldVelocityZ;       // Velocity in world space Z
  u_int16_t m_worldForwardDirX; // World space forward X direction (normalised)
  u_int16_t m_worldForwardDirY; // World space forward Y direction (normalised)
  u_int16_t m_worldForwardDirZ; // World space forward Z direction (normalised)
  u_int16_t m_worldRightDirX;   // World space right X direction (normalised)
  u_int16_t m_worldRightDirY;   // World space right Y direction (normalised)
  u_int16_t m_worldRightDirZ;   // World space right Z direction (normalised)
  float m_gForceLateral;        // Lateral G-Force component
  float m_gForceLongitudinal;   // Longitudinal G-Force component
  float m_gForceVertical;       // Vertical G-Force component
  float m_yaw;                  // Yaw angle in radians
  float m_pitch;                // Pitch angle in radians
  float m_roll;                 // Roll angle in radians
} typedef CarMotionData;

struct PacketMotionData
{
  PacketHeader m_header; // Header

  CarMotionData m_carMotionData[20]; // Data for all cars on track

  // Extra player car ONLY data
  float m_suspensionPosition[4];     // Note: All wheel arrays have the following order:
  float m_suspensionVelocity[4];     // RL, RR, FL, FR
  float m_suspensionAcceleration[4]; // RL, RR, FL, FR
  float m_wheelSpeed[4];             // Speed of each wheel
  float m_wheelSlip[4];              // Slip ratio for each wheel
  float m_localVelocityX;            // Velocity in local space
  float m_localVelocityY;            // Velocity in local space
  float m_localVelocityZ;            // Velocity in local space
  float m_angularVelocityX;          // Angular velocity x-component
  float m_angularVelocityY;          // Angular velocity y-component
  float m_angularVelocityZ;          // Angular velocity z-component
  float m_angularAccelerationX;      // Angular velocity x-component
  float m_angularAccelerationY;      // Angular velocity y-component
  float m_angularAccelerationZ;      // Angular velocity z-component
  float m_frontWheelsAngle;          // Current front wheels angle in radians
} typedef PacketMotionData;

// SESSION PACKET
// The session packet includes details about the current session in progress.
// Frequency: 2 per second
// Size: 147 bytes

struct MarshalZone
{
  float m_zoneStart; // Fraction (0..1) of way through the lap the marshal zone starts
  int8_t m_zoneFlag; // -1 = invalid/unknown, 0 = none, 1 = green, 2 = blue, 3 = yellow, 4 = red
} typedef MarshalZone;

struct PacketSessionData
{
  PacketHeader m_header; // Header

  u_int8_t m_weather;             // Weather - 0 = clear, 1 = light cloud, 2 = overcast
                                  // 3 = light rain, 4 = heavy rain, 5 = storm
  int8_t m_trackTemperature;      // Track temp. in degrees celsius
  int8_t m_airTemperature;        // Air temp. in degrees celsius
  u_int8_t m_totalLaps;           // Total number of laps in this race
  u_int16_t m_trackLength;        // Track length in metres
  u_int8_t m_sessionType;         // 0 = unknown, 1 = P1, 2 = P2, 3 = P3, 4 = Short P
                                  // 5 = Q1, 6 = Q2, 7 = Q3, 8 = Short Q, 9 = OSQ
                                  // 10 = R, 11 = R2, 12 = Time Trial
  int8_t m_trackId;               // -1 for unknown, 0-21 for tracks, see appendix
  u_int8_t m_era;                 // Era, 0 = modern, 1 = classic
  u_int16_t m_sessionTimeLeft;    // Time left in session in seconds
  u_int16_t m_sessionDuration;    // Session duration in seconds
  u_int8_t m_pitSpeedLimit;       // Pit speed limit in kilometres per hour
  u_int8_t m_gamePaused;          // Whether the game is paused
  u_int8_t m_isSpectating;        // Whether the player is spectating
  u_int8_t m_spectatorCarIndex;   // Index of the car being spectated
  u_int8_t m_sliProNativeSupport; // SLI Pro support, 0 = inactive, 1 = active
  u_int8_t m_numMarshalZones;     // Number of marshal zones to follow
  MarshalZone m_marshalZones[21]; // List of marshal zones – max 21
  u_int8_t m_safetyCarStatus;     // 0 = no safety car, 1 = full safety car
                                  // 2 = virtual safety car
  u_int8_t m_networkGame;         // 0 = offline, 1 = online
} typedef PacketSessionData;

// LAP DATA PACKET
// The lap data packet gives details of all the cars in the session.
// Frequency: Rate as specified in menus
// Size: 841 bytes

struct LapData
{
  float m_lastLapTime;          // Last lap time in seconds
  float m_currentLapTime;       // Current time around the lap in seconds
  float m_bestLapTime;          // Best lap time of the session in seconds
  float m_sector1Time;          // Sector 1 time in seconds
  float m_sector2Time;          // Sector 2 time in seconds
  float m_lapDistance;          // Distance vehicle is around current lap in metres – could
                                // be negative if line hasn’t been crossed yet
  float m_totalDistance;        // Total distance travelled in session in metres – could
                                // be negative if line hasn’t been crossed yet
  float m_safetyCarDelta;       // Delta in seconds for safety car
  u_int8_t m_carPosition;       // Car race position
  u_int8_t m_currentLapNum;     // Current lap number
  u_int8_t m_pitStatus;         // 0 = none, 1 = pitting, 2 = in pit area
  u_int8_t m_sector;            // 0 = sector1, 1 = sector2, 2 = sector3
  u_int8_t m_currentLapInvalid; // Current lap invalid - 0 = valid, 1 = invalid
  u_int8_t m_penalties;         // Accumulated time penalties in seconds to be added
  u_int8_t m_gridPosition;      // Grid position the vehicle started the race in
  u_int8_t m_driverStatus;      // Status of driver - 0 = in garage, 1 = flying lap
                                // 2 = in lap, 3 = out lap, 4 = on track
  u_int8_t m_resultStatus;      // Result status - 0 = invalid, 1 = inactive, 2 = active
                                // 3 = finished, 4 = disqualified, 5 = not classified
                                // 6 = retired
} typedef LapData;

struct PacketLapData
{
  PacketHeader m_header; // Header
  LapData m_lapData[20]; // Lap data for all cars on track
} typedef PacketLapData;

// EVENT PACKET
// This packet gives details of events that happen during the course of the race.
// Frequency: When the event occurs
// Size: 25 bytes

struct PacketEventData
{
  PacketHeader m_header;         // Header
  u_int8_t m_eventStringCode[4]; // Event string code, see above
};

// PARTICIPANTS PACKET
// This is a list of participants in the race. If the vehicle is controlled by AI, then the name will be the driver name. If this is a multiplayer game, the names will be the Steam Id on PC, or the LAN name if appropriate. On Xbox One, the names will always be the driver name, on PS4 the name will be the LAN name if playing a LAN game, otherwise it will be the driver name.
// Frequency: Every 5 seconds
// Size: 1082 bytes

struct ParticipantData
{
  u_int8_t m_aiControlled; // Whether the vehicle is AI (1) or Human (0) controlled
  u_int8_t m_driverId;     // Driver id - see appendix
  u_int8_t m_teamId;       // Team id - see appendix
  u_int8_t m_raceNumber;   // Race number of the car
  u_int8_t m_nationality;  // Nationality of the driver
  char m_name[48];         // Name of participant in UTF-8 format – null terminated
                           // Will be truncated with … (U+2026) if too long
} typedef ParticipantData;

struct PacketParticipantsData
{
  PacketHeader m_header; // Header

  u_int8_t m_numCars; // Number of cars in the data
  ParticipantData m_participants[20];
} typedef PacketParticipantsData;

// CAR SETUPS PACKET
// This packet details the car setups for each vehicle in the session. Note that in multiplayer games, other player cars will appear as blank, you will only be able to see your car setup and AI cars.
// Frequency: Every 5 seconds
// Size: 841 bytes

struct CarSetupData
{
  u_int8_t m_frontWing;             // Front wing aero
  u_int8_t m_rearWing;              // Rear wing aero
  u_int8_t m_onThrottle;            // Differential adjustment on throttle (percentage)
  u_int8_t m_offThrottle;           // Differential adjustment off throttle (percentage)
  float m_frontCamber;              // Front camber angle (suspension geometry)
  float m_rearCamber;               // Rear camber angle (suspension geometry)
  float m_frontToe;                 // Front toe angle (suspension geometry)
  float m_rearToe;                  // Rear toe angle (suspension geometry)
  u_int8_t m_frontSuspension;       // Front suspension
  u_int8_t m_rearSuspension;        // Rear suspension
  u_int8_t m_frontAntiRollBar;      // Front anti-roll bar
  u_int8_t m_rearAntiRollBar;       // Front anti-roll bar
  u_int8_t m_frontSuspensionHeight; // Front ride height
  u_int8_t m_rearSuspensionHeight;  // Rear ride height
  u_int8_t m_brakePressure;         // Brake pressure (percentage)
  u_int8_t m_brakeBias;             // Brake bias (percentage)
  float m_frontTyrePressure;        // Front tyre pressure (PSI)
  float m_rearTyrePressure;         // Rear tyre pressure (PSI)
  u_int8_t m_ballast;               // Ballast
  float m_fuelLoad;                 // Fuel load
} typedef CarSetupData;

struct PacketCarSetupData
{
  PacketHeader m_header; // Header
  CarSetupData m_carSetups[20];
} typedef PacketCarSetupData;

// CAR TELEMETRY PACKET
// This packet details telemetry for all the cars in the race. It details various values that would be recorded on the car such as speed, throttle application, DRS etc.
// Frequency: Rate as specified in menus
// Size: 1085 bytes

struct CarTelemetryData
{
  u_int16_t m_speed;                      // Speed of car in kilometres per hour
  u_int8_t m_throttle;                    // Amount of throttle applied (0 to 100)
  int8_t m_steer;                         // Steering (-100 (full lock left) to 100 (full lock right))
  u_int8_t m_brake;                       // Amount of brake applied (0 to 100)
  u_int8_t m_clutch;                      // Amount of clutch applied (0 to 100)
  int8_t m_gear;                          // Gear selected (1-8, N=0, R=-1)
  u_int16_t m_engineRPM;                  // Engine RPM
  u_int8_t m_drs;                         // 0 = off, 1 = on
  u_int8_t m_revLightsPercent;            // Rev lights indicator (percentage)
  u_int16_t m_brakesTemperature[4];       // Brakes temperature (celsius)
  u_int16_t m_tyresSurfaceTemperature[4]; // Tyres surface temperature (celsius)
  u_int16_t m_tyresInnerTemperature[4];   // Tyres inner temperature (celsius)
  u_int16_t m_engineTemperature;          // Engine temperature (celsius)
  float m_tyresPressure[4];               // Tyres pressure (PSI)
} typedef CarTelemetryData;

struct PacketCarTelemetryData
{
  PacketHeader m_header; // Header
  CarTelemetryData m_carTelemetryData[20];
  u_int32_t m_buttonStatus; // Bit flags specifying which buttons are being
                            // pressed currently - see appendices
} typedef PacketCarTelemetryData;

// CAR STATUS PACKET
// This packet details car statuses for all the cars in the race. It includes values such as the damage readings on the car.
// Frequency: 2 per second
// Size: 1061 bytes

struct CarStatusData
{
  u_int8_t m_tractionControl;      // 0 (off) - 2 (high)
  u_int8_t m_antiLockBrakes;       // 0 (off) - 1 (on)
  u_int8_t m_fuelMix;              // Fuel mix - 0 = lean, 1 = standard, 2 = rich, 3 = max
  u_int8_t m_frontBrakeBias;       // Front brake bias (percentage)
  u_int8_t m_pitLimiterStatus;     // Pit limiter status - 0 = off, 1 = on
  float m_fuelInTank;              // Current fuel mass
  float m_fuelCapacity;            // Fuel capacity
  u_int16_t m_maxRPM;              // Cars max RPM, point of rev limiter
  u_int16_t m_idleRPM;             // Cars idle RPM
  u_int8_t m_maxGears;             // Maximum number of gears
  u_int8_t m_drsAllowed;           // 0 = not allowed, 1 = allowed, -1 = unknown
  u_int8_t m_tyresWear[4];         // Tyre wear percentage
  u_int8_t m_tyreCompound;         // Modern - 0 = hyper soft, 1 = ultra soft
                                   // 2 = super soft, 3 = soft, 4 = medium, 5 = hard
                                   // 6 = super hard, 7 = inter, 8 = wet
                                   // Classic - 0-6 = dry, 7-8 = wet
  u_int8_t m_tyresDamage[4];       // Tyre damage (percentage)
  u_int8_t m_frontLeftWingDamage;  // Front left wing damage (percentage)
  u_int8_t m_frontRightWingDamage; // Front right wing damage (percentage)
  u_int8_t m_rearWingDamage;       // Rear wing damage (percentage)
  u_int8_t m_engineDamage;         // Engine damage (percentage)
  u_int8_t m_gearBoxDamage;        // Gear box damage (percentage)
  u_int8_t m_exhaustDamage;        // Exhaust damage (percentage)
  int8_t m_vehicleFiaFlags;        // -1 = invalid/unknown, 0 = none, 1 = green
                                   // 2 = blue, 3 = yellow, 4 = red
  float m_ersStoreEnergy;          // ERS energy store in Joules
  u_int8_t m_ersDeployMode;        // ERS deployment mode, 0 = none, 1 = low, 2 = medium
                                   // 3 = high, 4 = overtake, 5 = hotlap
  float m_ersHarvestedThisLapMGUK; // ERS energy harvested this lap by MGU-K
  float m_ersHarvestedThisLapMGUH; // ERS energy harvested this lap by MGU-H
  float m_ersDeployedThisLap;      // ERS energy deployed this lap
} typedef CarStatusData;

struct PacketCarStatusData
{
  PacketHeader m_header; // Header
  CarStatusData m_carStatusData[20];
} typedef PacketCarStatusData;

G_END_DECLS

#endif /* __GST_FORMULAONEPARSE_H__ */
