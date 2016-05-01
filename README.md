# NS3 Project - ET 4394

------
by

 Kangqi Li
 
 4518942

This is the repo for the NS3 project assignment. There is a PDF file as the final result and a .cc file as the source code used. This source code can be built and run by ./waf of NS3.

Example: 
         
         $ ./waf --run "scratch/ProjectNS3 --ps=16.0206 --distance=900"
       
         $ ./waf --run "scratch/ProjectNS3 --ps=16.0206 --distance=900"
         
         

My original proposed scenario is about the power of PHY but I changed my original idea since I think this objective is more interesting. So my final work is not about the proposed scenario that I told to you two months ago. Hope this won't be a problem.

It is required to build script to generate all the results automatically. However, there is no function of NS3 that can find the wifi range. So the source code can only track the throughput, delay and packet loss at specific distance. Then the wifi range is detected manually. Hence I couldn't build the script. Very sorry for this. Hope this won't disturb you too much.
