
#ifndef GLOBAL_MACROS_H_
#define GLOBAL_MACROS_H_

#define RAY_TRACING_ENABLED true
#define PROFILING true

#define WORK_GROUP_SIZE 16
#define NUM_GROUPS_X (windowWidth/WORK_GROUP_SIZE)
#define NUM_GROUPS_Y (windowHeight/WORK_GROUP_SIZE)

#define GPU_ALIGNMENT (4 * sizeof(uint32_t)) // 32 byte aligned

#define ASSERT_GPU_ALIGNMENT(struct_name)\
	static_assert(\
		(sizeof(struct_name) % GPU_ALIGNMENT) == 0,\
		#struct_name " not GPU aligned.")

#define ASSERT_STRUCT_UP_TO_DATE(struct_name, size)\
	static_assert(\
		sizeof(struct_name) == size,\
		"Don't forget to update this function and mirrored structs in shaders if added/removed member variables from " #struct_name)

#endif