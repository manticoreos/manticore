Research
---------

The paper was published in HOTOS'19 conference with the title

I/O Is Faster Than the CPU – Let’s Partition Resources
and Eliminate (Most) OS Abstractions

The paper cover the scenarios when I/O is faster than the CPU. There were times
in 1970 during mainframes when the mainframe pheripherals can't connect with the
mainmeory. The transfer directly happens using channels. This scenarios was
different from a microservers.

.. image:: /docs/guides/research/1.png

Multiple server applications are isolated for reducing the layers in the parakernel.

There are 4 bad scenarios that are the issue we worked in removing in the papers:
- Limited I/O performance
- Limited application level parallelism
- Predictability of tail latency
- Predictablility of energy

.. image:: /docs/guides/research/2.png

We partition the kernel hardware much like MICA and SeaStar implementation. We
then remove the POSIX API standards and implementation. The parakernel would have
a:

- A user space
- A kernel space
- NIC having Rx/Tx pair

.. image:: /docs/guides/research/3.png

The open area of research and use of parakernels are in serverless architecture.
Most contains are heavy weight and hence remains in the papers exploration list.

.. image:: /docs/guides/research/4.png
