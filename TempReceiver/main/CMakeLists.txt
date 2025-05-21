idf_component_register(SRCS 
                            "main.c"
                            "utils/CAN/can_driver_utils.c"
                            "utils/CAN/can_receive_utils.c"
                    INCLUDE_DIRS 
                            "." 
                            "utils/CAN")