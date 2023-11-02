# jvmti agent

jvmti agent, to map memory objects to csv

## doc

- [oracle jdk jvmti doc](https://docs.oracle.com/javase/8/docs/platform/jvmti/jvmti.html)
- [ibm jdk jvmti doc](https://www.ibm.com/docs/en/sdk-java-technology/8?topic=interfaces-jvmti)

## build

need to install xmake first.

- build

    ```shell
    xmake
    ```

- build debug

    ```shell
    xmake config -m debug
    xmake
    ```

- build release


    ```shell
    xmake config -m release
    xmake
    ```

