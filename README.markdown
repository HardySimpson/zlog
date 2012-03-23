# What is zlog? #

*zlog* is a high efficent, thread safe, flexsible, clear model, pure c logging library

Actually, in the c world there is NO good logging library for application like logback in java or log4cxx in c++. printf can work, but can not be easily redirected or reformat, syslog is slow and is designed for system use.

So I write *zlog*. 

*zlog* has feartures below:

  * syslog style configure file, easy for understand and use
  * range-category model, which is more flexible and more clear than hierarchy model of log4j
  * multiple output, include static file path, dynamic file path, stdout, stderr, syslog
  * runtime refreash configure to change output flow or output format, just need to call one function -- zlog_update()
  * high efficieny, on my laptop, record 720'000 log per second, about 200 times faster than syslog(3) with rsyslogd
  * user can define his own log level without change and rebuild library[sec:Define-new-level]
  * safely rotate log file by size when multiple process or multiple threads write to one same log file
  * accurate to microseconds
  * MDC, a log4j style key-value map, expand user defined field in dynamic log file path or log format, is also useful in multi-thread programming
  * self debugable, can output zlog's self debug and error log at runtime
  * Not depend on any other 3rd party library, just base on POSIX system.

Links:

[Users Guide in English](https://github.com/HardySimpson/zlog/raw/master/download/ZlogUsersGuide-EN-0.9rc1.pdf)

[Users Guide in Chiniese](https://github.com/HardySimpson/zlog/raw/master/download/ZlogUsersGuide-CN-0.9rc1.pdf)

[Author's Blog](http://my.oschina.net/HardySimpson/blog)

[Author's Email](mailto:HardySimpson1984@gmail.com)
