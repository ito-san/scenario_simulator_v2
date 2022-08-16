ARG ROS_DISTRO
FROM ros:${ROS_DISTRO}
SHELL ["/bin/bash", "-c"]
ENV DEBIAN_FRONTEND=noninteractive
ENV DEBCONF_NOWARNINGS=yes
ARG ROS_DISTRO

RUN sudo apt-get update && sudo apt-get -y install python3-pip python3-rospkg python3-rosdep software-properties-common
RUN add-apt-repository ppa:kisak/kisak-mesa -y
RUN apt-get update && apt-get install libegl-mesa0 libglfw3-dev -y

WORKDIR /home/ubuntu/Desktop/scenario_simulator_ws/src/scenario_simulator
COPY . $WORKDIR

RUN mv oneTBB /home/ubuntu/Desktop/ && mv embree /home/ubuntu/Desktop/
WORKDIR /home/ubuntu/Desktop/oneTBB
RUN mkdir build && cd build && cmake -DCMAKE_CXX_STANDARD=20 -DCMAKE_BUILD_TYPE=debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DTBB_CPF='ON' -DTBB_TEST=OFF .. && make && make install
WORKDIR /home/ubuntu/Desktop/embree
RUN mkdir build && cd build && cmake ../ -DEMBREE_ISPC_SUPPORT=OFF && make && make install

WORKDIR /home/ubuntu/Desktop/scenario_simulator_ws/
RUN mkdir -p /home/ubuntu/Desktop/scenario_simulator_ws/src/scenario_simulator/external
WORKDIR /home/ubuntu/Desktop/scenario_simulator_ws/src/scenario_simulator
RUN vcs import external < dependency_${ROS_DISTRO}.repos
WORKDIR /home/ubuntu/Desktop/scenario_simulator_ws/src
RUN source /opt/ros/${ROS_DISTRO}/setup.bash && rosdep install -iry --from-paths . --rosdistro ${ROS_DISTRO}  --skip-keys embree

WORKDIR /home/ubuntu/Desktop/scenario_simulator_ws
RUN source /opt/ros/${ROS_DISTRO}/setup.bash && MAKEFLAGS=-j3 colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release --parallel-worker 3
COPY ./docker-entrypoint.sh /
RUN chmod a+x /docker-entrypoint.sh

ENTRYPOINT ["/docker-entrypoint.sh"]
