
#ifndef GLOBAL_MACROS_H_
#define GLOBAL_MACROS_H_

#define RAY_TRACING_ENABLED true
#define PROFILING true

#define WORK_GROUP_SIZE_X 16
#define WORK_GROUP_SIZE_Y 16
#define NUM_GROUPS_X (windowWidth/WORK_GROUP_SIZE_X)
#define NUM_GROUPS_Y (windowHeight/WORK_GROUP_SIZE_Y)

#define ASSERT_GPU_ALIGNMENT(struct_name, value)\
	static_assert(\
		(sizeof(struct_name) % value) == 0,\
		#struct_name " not GPU aligned.")

#define ASSERT_STRUCT_UP_TO_DATE(struct_name, size)\
	static_assert(\
		sizeof(struct_name) == size,\
		"Don't forget to update this function and mirrored structs in shaders if added/removed member variables from " #struct_name)

#endif