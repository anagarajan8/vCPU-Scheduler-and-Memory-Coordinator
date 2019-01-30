# vCPU-Scheduler-and-Memory-Coordinator

Implemented a vCPU scheduler and a memory coordinator to dynamically manage the resources assigned to each guest machine. Each of these programs runs in the host machine's user space, collects statistics for each guest machine through hypervisor calls and takes proper actions. Both programs are independent of each other.

In every stat collection cycle, the vCPU scheduler tracks each guest machine's vCpu utilization, and decides how to pin them to pCpus, so that all pCpus are "balanced", where every pCpu handles a similar amount of workload.

Similarly, in every stat collection cycle, the memory coordinator tracks each guest machine's memory utilization, and decides how much extra free memory should be given to each guest machine. The memory coordinator sets the memory size of each guest machine and triggers the balloon driver to inflate and deflate. The memory coordinator reacts when the memory resource is insufficient.

Tools used: 1 . qemu-kvm, libvirt-bin, libvirt-dev are packages required to install to launch virtual machines with KVM and develop programs to manage virtual machines. 2. libvirt is a toolkit providing lots of APIs to interact with the virtualization capabilities of Linux. 3. virsh, uvtool, virt-top, virt-clone, virt-manager, virt-install are tools that helps with virtual machine interactions.

Machine Used for testing: VMware Fusion (4GB Host, Ubuntu 16.04) on MAC OS
