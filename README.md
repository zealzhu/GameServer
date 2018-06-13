# QQ群消息收集服务器

#### 项目介绍
    用来将QQ群消息收集到mysql服务器用以分析

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