# qrexec-c-adapter
This is a stripped down version of qrexec-client-vm service that can be used as a library for C/C++

## Building
```bash
mkdir build && cd build && cmake .. && make -j
```

## Checking out example
1. Prepare the environment according to the [offitial guide](https://www.qubes-os.org/doc/qrexec/) tldr:
   * Set up destination Qubes VM (`RPCTest` in example.cpp)
   * Add the following line to `/etc/qubes-rpc/policy/test.Add` in dom0:
     ```
     @anyvm @anyvm allow
     ```
   * Add the following python script in destination VM to `/usr/bin/test_add_server`:
     ```python3
     #!/bin/python3
     a, b = map(int, input().split())
     print(a + b)
     ```
   * Create symlink in destination VM:
     ```bash
     sudo ln -s /usr/bin/test_add_server /etc/qubes-rpc/test.Add
     ```
2. Build example
3. Run `example` binary

## Using in an actual project
For now, the library is distributed only as CMake project. Maybe I will have time to make it header-only in the future, but no promises there.
See how to include the lib in CMake in [real example](https://github.com/Lord-KA/GUILib-isolation-plug/blob/master/CMakeLists.txt).
