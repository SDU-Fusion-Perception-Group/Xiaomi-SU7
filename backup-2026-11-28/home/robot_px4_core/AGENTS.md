# robot_px4_core
C++17 on Jetson/Linux; drivers own I/O and publish typed snapshots. One producer and one consumer per topic. No mutex in topic transport; use atomic slot ownership and drop writes if needed. Drivers request CPU 0 affinity; report scheduling failures, never require real-time privileges. Preserve SI units and monotonic timestamps. Never guess sensor udev mappings.
