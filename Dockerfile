FROM osrf/space-ros

# Install dependencies of the ROS - cFS bridge
RUN sudo apt-get install -y libwebsockets-dev protobuf-c-compiler libprotobuf-c-dev
RUN sudo apt-get install -y pip
RUN pip install protobuf websockets

# Install other dev tools
RUN sudo apt-get install -y git

# This is strictly not needed, but makes debugging and demonstration easier
RUN sudo apt-get install -y tmux vim gdb

# Clone bridge first. The bridge has both cFS and ROS content we need before we
# compile either.
WORKDIR ${HOME_DIR}
# RUN git clone https://github.com/jaxa/racs2_bridge

# Prepare cFS

## Clone cFS
WORKDIR ${HOME_DIR}
RUN git clone --recursive -b v6.7.0a https://github.com/nasa/cFS/ cfs
WORKDIR ${HOME_DIR}/cfs
RUN git submodule init
RUN git submodule update

## Customize cFS to run the bridge
RUN cp cfe/cmake/Makefile.sample Makefile
RUN cp -r cfe/cmake/sample_defs sample_defs
# RUN cp -pr ${HOME_DIR}/racs2_bridge/cFS/Bridge/Client_C/apps/racs2_bridge_client ${HOME_DIR}/cfs/apps/
COPY ./cFS/Bridge/Client_C/apps/racs2_bridge_client ${HOME_DIR}/cfs/apps/racs2_bridge_client
# The following are the sample_defs needed if we only want the bridge and not the sample app.
# RUN cp -p ${HOME_DIR}/racs2_bridge/cFS/Bridge/Client_C/sample_defs/* ${HOME_DIR}/cfs/sample_defs/

## Deploy the event talker application and adjust the startup scripts.
# RUN cp -pr ${HOME_DIR}/racs2_bridge/Example/Case.3/cFS/sample_defs/* ${HOME_DIR}/cfs/sample_defs/
COPY ./Example/Case.3/cFS/sample_defs ${HOME_DIR}/cfs/sample_defs/
# RUN cp -pr ${HOME_DIR}/racs2_bridge/Example/Case.3/cFS/apps/event_talker ${HOME_DIR}/cfs/apps/
COPY ./Example/Case.3/cFS/apps/event_talker ${HOME_DIR}/cfs/apps/event_talker

## This is necessary to run cFS inside docker, apparently.
RUN sed -i -e 's/^#undef OSAL_DEBUG_PERMISSIVE_MODE/#define OSAL_DEBUG_PERMISSIVE_MODE 1/g' sample_defs/default_osconfig.h
RUN sed -i -e 's/^#undef OSAL_DEBUG_DISABLE_TASK_PRIORITIES/#define OSAL_DEBUG_DISABLE_TASK_PRIORITIES 1/g' sample_defs/default_osconfig.h

## This is only needed because docker by default starts in IPv4. This setting
## is specific to the JAXA bridge.
RUN sed -i -e 's/^wss_uri=.*/wss_uri=127.0.0.1/g' sample_defs/racs2_bridge_config.txt

## Compile cFS
RUN make SIMULATION=native prep
RUN make
RUN make install

# Prepare ROS packages

## Create ROS workspace
WORKDIR ${HOME_DIR}
RUN mkdir -p ros2-project/src

## Copy packages (bridge and demo listener).
# RUN cp -pr ${HOME_DIR}/racs2_bridge/ROS2/Bridge/Server_Python/bridge_py_s ${HOME_DIR}/ros2-project/src/
COPY ./ROS2/Bridge/Server_Python/bridge_py_s ${HOME_DIR}/ros2-project/src/bridge_py_s
# RUN cp -pr ${HOME_DIR}/racs2_bridge/Example/Case.3/ROS2/* ${HOME_DIR}/ros2-project/src/
COPY ./Example/Case.3/ROS2 ${HOME_DIR}/ros2-project/src/

## Compile and install ROS 2 packages
WORKDIR ${HOME_DIR}/ros2-project
SHELL ["/bin/bash", "-c"]
RUN source ${SPACEROS_DIR}/install/setup.bash && colcon build --symlink-install

## This is only needed because docker by default starts in IPv4. This setting
## is specific to the JAXA bridge.
RUN sudo sed -i -e 's/wss_uri:.*/wss_uri: "127.0.0.1"/g' ./src/bridge_py_s/config/params.yaml

WORKDIR ${HOME_DIR}

