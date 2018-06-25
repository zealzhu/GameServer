# 游戏通用服务器

#### 项目介绍
    用来快熟搭建游戏服务器，只需在module中添加游戏业务模块。里面现有模块是一个用来收集qq群消息并存储到mysql中

#### 软件架构
    自用架构


#### 安装教程

## linux环境下依赖
    protobuf
    mysql

## linux
    mkdir build && cd build
    cmake ..
    make

## windows
    安装cmake使用cmake图形化界面生成vs解决方案

#### 使用说明
    需配合酷Q实现的客户端使用
    如需添加模块则拷贝module目录下的随意一个模块进行修改CMakeLists, 如需加载模块在config.ini配置中添加模块名以;结束
