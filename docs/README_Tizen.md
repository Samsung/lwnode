# Introduction
The development of node.js has created a platform-independent JavaScript runtime environment, where Web developers can run their JavaScript code outside a Web browser. Benefits of using node.js for a Web app developer when creating Web apps includes, but not limited to, reusing already known Web development knowledge, using only JavaScript on both server and client sides, and so on. While node.js provides the state-of-the-art performance and a wide range of supporting libraries, it is not suitable for memory-constrained devices such as TVs, and consumer electronics. This is because node.js is designed to provide better performance by efficiently using a large amount of memory. This goal of node.js conflicts with the goal of optimizing memory usage on these devices in memory-constrained environments. Adopting node.js to these memory-constrained devices is challenging, because the design and implementation of node.js conflict with the physically small amount of memory shipped with these devices.

In this document, we introduce lightweight node.js (LWNode) whose aim is to provide a JavaScript runtime environment for memory-constrained devices. Some advantages of LWNode are as follows:

* LWNode is a memory-optimized node.js for Tizen-based memory-constrained devices such as TVs. Compared to stock node.js, LWNode consumes 30% less amount of memory on TV.
* Supports a wide range of node.js modules

LWNode is already shipped to the following Samsung products.

* Provides a runtime environment for Spotify on TV

# Expected Values
LWNode can easily provide a JavaScript runtime environment in a memory-efficient way. In particular, LWNode

* Provides a Tizen-optimized, and memory-efficient JavaScript runtime environment
* Provides a set of Tizen-specific Web Device APIs

# Requirements
LWNode is designed to meet the following requirements.

* Functional requirements
  - Supports Spotify on TV
  - Supports a set of node.js modules defined in Appendix 1
  - Supports a set of Tizen Web Device APIs
* Non-functional requirements
  - Customizable for various business requirements and target devices
  - Low memory usage and small memory footprints

# Target Applications
LWNode supports the following devices.

* Tizen-based devices such as TVs, speakers, etc.

# Appendix 1: Supporting Modules
A list of supporting modules are described in [spec.md](spec.md).
