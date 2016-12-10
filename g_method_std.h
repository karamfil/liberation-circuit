
#ifndef H_G_METHOD_STD
#define H_G_METHOD_STD

s16b call_std_method(struct core_struct* core, int call_value, int variable_parameters);

void find_nearby_well(struct core_struct* core);
s16b lower_general_interface(struct core_struct* core);

s16b scan_for_auto_attack(struct core_struct* core, int angle, int scan_distance, int target_index);

int check_static_build_location_for_data_wells(al_fixed build_x, al_fixed build_y);


#define SMETHOD_VARIABLE_PARAMS_MAX 16
// this is the most parameters a call with a variable number of parameters will accept

enum
{
SMETHOD_CALL_SCAN_FOR_THREAT,
SMETHOD_CALL_CHECK_POINT,
SMETHOD_CALL_CHECK_XY_VISIBLE,
SMETHOD_CALL_GET_COMMAND_TYPE,
SMETHOD_CALL_GET_COMMAND_X,
SMETHOD_CALL_GET_COMMAND_Y,
SMETHOD_CALL_GET_COMMAND_NUMBER,
SMETHOD_CALL_GET_COMMAND_CTRL,
SMETHOD_CALL_GET_COMMANDS,
SMETHOD_CALL_CLEAR_COMMAND,
SMETHOD_CALL_CLEAR_ALL_COMMANDS,
SMETHOD_CALL_GET_COMMAND_TARGET,
SMETHOD_CALL_GET_COMMAND_TARGET_COMPONENT,
SMETHOD_CALL_CHECK_NEW_COMMAND,
/*

SMETHOD_CALL_CHECK_NEW_BUILD_COMMAND,
SMETHOD_CALL_CHECK_BUILD_COMMAND,
SMETHOD_CALL_GET_BUILD_COMMAND_X,
SMETHOD_CALL_GET_BUILD_COMMAND_Y,
SMETHOD_CALL_GET_BUILD_COMMAND_ANGLE,
SMETHOD_CALL_GET_BUILD_COMMAND_TEMPLATE,
SMETHOD_CALL_GET_BUILD_COMMAND_CTRL,
SMETHOD_CALL_CLEAR_BUILD_COMMAND,
SMETHOD_CALL_CLEAR_ALL_BUILD_COMMANDS,*/

SMETHOD_CALL_BUILD_FROM_QUEUE,
SMETHOD_CALL_CHECK_BUILD_QUEUE,
SMETHOD_CALL_CHECK_BUILD_QUEUE_FRONT,
SMETHOD_CALL_ADD_TO_BUILD_QUEUE,
SMETHOD_CALL_CANCEL_BUILD_QUEUE,
SMETHOD_CALL_BUILD_QUEUE_GET_TEMPLATE,
SMETHOD_CALL_BUILD_QUEUE_GET_X,
SMETHOD_CALL_BUILD_QUEUE_GET_Y,
SMETHOD_CALL_BUILD_QUEUE_GET_ANGLE,


//SMETHOD_CALL_GET_INTERFACE_STRENGTH, - now a core method
//SMETHOD_CALL_GET_INTERFACE_CAPACITY,
SMETHOD_CALL_CHARGE_INTERFACE,
SMETHOD_CALL_SET_INTERFACE_GENERAL,
//SMETHOD_CALL_SPARE_POWER_TO_INTERFACE,
//SMETHOD_CALL_ALL_POWER_TO_INTERFACE,
SMETHOD_CALL_CHARGE_INTERFACE_MAX,

SMETHOD_CALL_CHECK_SELECTED,
SMETHOD_CALL_CHECK_SELECTED_SINGLE,

SMETHOD_CALL_GET_AVAILABLE_DATA,
SMETHOD_CALL_SEARCH_FOR_WELL,
SMETHOD_CALL_GET_WELL_X,
SMETHOD_CALL_GET_WELL_Y,
SMETHOD_CALL_GET_WELL_DATA,
SMETHOD_CALL_GET_DATA_STORED,
SMETHOD_CALL_GET_DATA_CAPACITY,
SMETHOD_CALL_SCAN_SINGLE,
SMETHOD_CALL_SCAN_MULTI,
//SMETHOD_CALL_SCAN_FOR_TEMPLATE,
SMETHOD_CALL_GET_POWER_CAPACITY,
SMETHOD_CALL_GET_POWER_USED,
SMETHOD_CALL_GET_POWER_LEFT,
//SMETHOD_CALL_GET_POWER_USED_ACTUAL,
//SMETHOD_CALL_GET_POWER_LEFT_ACTUAL,
//SMETHOD_CALL_GET_STRESS,
//SMETHOD_CALL_GET_STRESS_PERCENT,
SMETHOD_CALL_SET_DEBUG_MODE,

SMETHOD_CALL_TRANSMIT,// (target, channel, priority, <message...>) -3 means a variable number of parameters, with a minimum of 3 (should this be -4 instead)
SMETHOD_CALL_BROADCAST,// (range, channel, priority, <message...>)
SMETHOD_CALL_TRANSMIT_TARGET,// (target, channel, priority, target, <message...>)
SMETHOD_CALL_BROADCAST_TARGET,// (range, channel, priority, target, <message...>)
SMETHOD_CALL_CHECK_MESSAGES,
SMETHOD_CALL_GET_MESSAGE_TYPE,
SMETHOD_CALL_GET_MESSAGE_CHANNEL,
SMETHOD_CALL_GET_MESSAGE_SOURCE,// (target memory index)
SMETHOD_CALL_GET_MESSAGE_X,
SMETHOD_CALL_GET_MESSAGE_Y,
SMETHOD_CALL_GET_MESSAGE_TARGET,
SMETHOD_CALL_GET_MESSAGE_PRIORITY,
SMETHOD_CALL_READ_MESSAGE,
SMETHOD_CALL_NEXT_MESSAGE,
SMETHOD_CALL_IGNORE_CHANNEL,
SMETHOD_CALL_LISTEN_CHANNEL,
SMETHOD_CALL_IGNORE_ALL_CHANNELS,
SMETHOD_CALL_COPY_COMMANDS,
SMETHOD_CALL_GIVE_COMMAND,
SMETHOD_CALL_GIVE_BUILD_COMMAND,
SMETHOD_CALL_CHECK_BUILD_RANGE,

SMETHOD_CALL_REPAIR_SELF,
SMETHOD_CALL_RESTORE_SELF,

SMETHOD_CALL_REPAIR_OTHER,
SMETHOD_CALL_REPAIR_SCAN,
SMETHOD_CALL_RESTORE_OTHER,
SMETHOD_CALL_RESTORE_SCAN,

SMETHOD_CALL_BUILD_PROCESS, // (template_index, x_offset, y_offset, angle, target address);
//SMETHOD_CALL_BUILD_AS_COMMANDED, // (target address)
SMETHOD_CALL_BUILD_REPEAT, // (target address)
SMETHOD_CALL_GET_TEMPLATE_COST,
SMETHOD_CALL_RANDOM,

SMETHOD_CALL_CHECK_CONTACT,
SMETHOD_CALL_GET_DAMAGE,
SMETHOD_CALL_GET_DAMAGE_SOURCE,
SMETHOD_CALL_DISTANCE_FROM_XY,
//SMETHOD_CALL_DISTANCE_FROM_XY_HYPOT,
SMETHOD_CALL_DISTANCE_FROM_XY_LESS,
SMETHOD_CALL_DISTANCE_FROM_XY_MORE,
SMETHOD_CALL_DISTANCE_XY,
//SMETHOD_CALL_DISTANCE_FROM_TARGET, // these are done as core methds
//SMETHOD_CALL_DISTANCE_FROM_TARGET_HYPOT,

SMETHOD_CALL_TARGET_CLEAR, // (target_index)
SMETHOD_CALL_TARGET_COMPARE, // (target1, target2)
SMETHOD_CALL_TARGET_COPY, // (target_dest, target_source)
SMETHOD_CALL_TARGET_DESTROYED, // (target_index)

SMETHOD_CALL_ATTACK_MODE,

SMETHOD_CALL_GET_PROCESS_COUNT,
SMETHOD_CALL_GET_PROCESSES_MAX,
SMETHOD_CALL_GET_PROCESSES_UNUSED,
SMETHOD_CALL_GET_COMPONENT_COUNT,
SMETHOD_CALL_GET_COMPONENTS_MAX,
SMETHOD_CALL_GET_COMPONENTS_UNUSED,
SMETHOD_CALL_SPECIAL_AI,

SMETHOD_CALL_TYPES
};

struct smethod_call_type_struct
{
	int parameters; // this is the number of parameters that will be pulled off the stack (in addition to the object/class index)
// parameters shouldn't be greater than CMETHOD_CALL_PARAMETERS
};


enum
{
//SCAN_BITFIELD_FRIENDLY, - dealt with separately
// - remember to do (1 << SCAN_BITFIELD_xx) when using
SCAN_BITFIELD_STATIC,
SCAN_BITFIELD_MOBILE, // really means non-static - might not have any move objects
SCAN_BITFIELD_OBJ_BUILD,
SCAN_BITFIELD_OBJ_ALLOCATE,
SCAN_BITFIELD_OBJ_GATHER,
SCAN_BITFIELD_OBJ_PULSE,
SCAN_BITFIELD_OBJ_SPIKE,
SCAN_BITFIELD_OBJ_STREAM,
SCAN_BITFIELD_OBJ_BURST,
SCAN_BITFIELD_OBJ_INTERFACE,
SCAN_BITFIELD_OBJ_ULTRA,
SCAN_BITFIELD_OBJ_SLICE,
};

#define SCANLIST_SIZE 128

struct scanlist_struct
{
	int current; // set to 0 at start of every execution, then to 1 when scan is run
	int list_size; // number of cores in list. unreliable unless current == 1
	int index [SCANLIST_SIZE]; // core index. Don't need to compare vs timestamp because scanlist only relevant over a single execution
	 // note that these lists are not necessarily sorted. They'll probably just be in index order.
	al_fixed core_x [SCANLIST_SIZE]; // store x/y values in scanlist
	al_fixed core_y [SCANLIST_SIZE];
	s16b scan_bitfield [SCANLIST_SIZE];

/*
scan_bitfield:

static
has build object
has allocator
has gather
has any packet
has spike
has stream
has interface

sorting:
 can sort up to 6? processes from near to far (simple x diff + y diff)

scan(x_offset, y_offset, target memory, accept_friendly, components_min, components_max, scan_bitfield)
scan_multi(x_offset, y_offset, target memory, number of targets, components_min, components_max, scan_bitfield)

//scan_complex(x_offset, y_offset, target memory, number of targets, want_bitfield, reject_bitfield)

*/

};


#endif
