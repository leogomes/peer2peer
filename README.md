p2p
=========

A peer to peer application written for a network programming course at university back in 2004. This is a backup I could find and certainly not the final version.

I will try to find a description of the application protocol used someday. I should have a LaTeX document with it somewhere. In short, it was a non-centralized peer to peer file sharing system, where the users are introduced via invitations. So, she needs to know the IP of at least two other peers that will introduce her to the network of peers. We will then try to find the closest neighbors, which are not necessarily the two nodes that introduced her to the network, and will connect with these two chosen ones. After that, she can start to search for files and that will be done via broadcast search messages with a maximum depth. Several positive responses can be received and the system will select the closest peer to start a file exchange.

To be completed.

The funniest part of this project was doing it in a non-blocking and asynchronous way. We had a lot of fun reading Steven's Unix Networking Programming as well. Good times :)

I wrote it mainly with [Gustavo Nasu](br.linkedin.com/pub/gustavo-nasu/7/895/645).

Ricardo Lilenbaum and Fabio Sarmento also participated to the group and to some discussions/coding, but the bulk of their contribution was more linked to the presentation and document we wrote about it. 
