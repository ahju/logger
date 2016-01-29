# logger
这是非常简单的C++日志记录模块，采用单实例模式开发，可以简化日志记录。不足的是无法在一个工程里记录多种日志

## 文件说明
1.logger.h    日志类头文件<br>
2.logger.cpp  日志类实现文件<br>
3.thread1.cpp 测试线程1<br>
4.thread2.cpp  测试线程2<br>
5.test_log.cpp  测试实现程序，使用多线程进行测试<br>
6.test_log_DEBUG.mk  测试工程的makefile文件<br>
7.test_log.prj  测试工程文件

## 使用说明
1.只需要在工程中创建一个对象，使用Logger::get_instance();获取对象即可使用<br>
2.分别使用logit_info，logit_warn，logit_error，logit_debug记录不同级别的日志<br>
3.编译工程是必须带上-lpthread
