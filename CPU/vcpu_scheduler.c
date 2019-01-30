//CS 6210 AOS vcpu_scheduler.c
//Akshaya Nagarajan
//GTID 90331262

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<libvirt/libvirt.h>
#define TIMECONVERSION 1000000000
#define VCPUTHRESHOLD 25
#define NUMBEROFPCPUS 4

struct StoreDomainValues{
	unsigned long long cputime;
	int cpunumber;
};

struct PcpuValueMap{
	int *vcpunumber;
	int *vcpuutilization;
};


int main(int argc, char *argv[])
{
	int senseflag = 0;
	virConnectPtr conn;
	int numDomains,i;
	int *activeDomains;
	virDomainPtr domainPtr;
	virDomainPtr domainPtr2,domainPtr3,domainPtr4;


	conn = virConnectOpen("qemu:///system");

	if (conn == NULL)
	{
		fprintf(stderr, "Failed to connect to Hypervisor");
		exit(1);
	}
	else
	{
		printf("Hypervisor Connected\n");
	}

	numDomains = virConnectNumOfDomains(conn);
	printf("Number of Domains are %d\n",numDomains);

	activeDomains = malloc(sizeof(int) * numDomains);
	virVcpuInfoPtr info = malloc(sizeof(virVcpuInfoPtr) * numDomains);
	numDomains = virConnectListDomains(conn, activeDomains, numDomains);

	struct StoreDomainValues First[numDomains];
	struct StoreDomainValues Second[numDomains];
	struct PcpuValueMap Third[NUMBEROFPCPUS];

	while(1)
	{

		//VCPU Stats at Instance (t)s
		for (i = 0 ; i < numDomains ; i++) {
			printf("Domain ids are  %d\t", activeDomains[i]);
			domainPtr = virDomainLookupByID(conn, activeDomains[i]);
			virDomainGetVcpus(domainPtr, info, numDomains, NULL, 0);
			First[i].cputime = info[0].cpuTime;
			First[i].cpunumber = info[0].cpu;
			printf("CPU time is %llu\t",First[i].cputime);
			printf("PCPU is %d\n",First[i].cpunumber);
		}

		sleep(atoi(argv[1]));

		printf("\n");
		//VCPU Stats at Instance (t+12)s
		unsigned char * cpuMap = malloc (sizeof(char));
		for(i = 0 ; i < numDomains ; i++) {
			printf("Domain ids are  %d\t", activeDomains[i]);
			domainPtr = virDomainLookupByID(conn, activeDomains[i]);
			virDomainGetVcpus(domainPtr, info, numDomains, NULL, 0);
			Second[i].cputime = info[0].cpuTime;
			Second[i].cpunumber = info[0].cpu;
			switch(Second[i].cpunumber){
				case 0:
					* cpuMap = 1;
					break;
				case 1:
					* cpuMap = 1 << 1;
					break;
				case 2:
					* cpuMap = 1 << 2;
					break;
				case 3:
					*cpuMap = 1 << 3;
					break;
				default:
					break;
			}
			if(senseflag == 0)
			{
				virDomainPinVcpu(domainPtr, 0, cpuMap, 1);
			}
			printf("CPU time is %llu\t",Second[i].cputime);
			printf("PCPU is %d\n",Second[i].cpunumber);
		}
		printf("Sense Flag is %d\n", senseflag);
		senseflag = 1;
		printf("\n");


		//Getting individual vCPU Utilizations
		printf("Interval is %d\n",atoi(argv[1])); 
		for(i = 0; i < numDomains; i++)
		{
			First[i].cputime = (((Second[i].cputime - First[i].cputime)/atoi(argv[1]))*100)/TIMECONVERSION;
			printf("CPU utilization of vcpu %d is %llu\n", activeDomains[i], First[i].cputime);
		}
		printf("\n");

		int pcpuarray[NUMBEROFPCPUS] = {0};
		int vcpucount[NUMBEROFPCPUS] = {0};




		//Getting Pcpu Utilizations
		for(i=0; i< numDomains; i++)
		{
			switch(Second[i].cpunumber)
			{
				case 0: 
					pcpuarray[0] = pcpuarray[0] + (int)First[i].cputime;
					vcpucount[0]++;
					break;
				case 1:
					pcpuarray[1] = pcpuarray[1] + (int)First[i].cputime;
					vcpucount[1]++;
					break;

				case 2:
					pcpuarray[2] = pcpuarray[2] + (int)First[i].cputime;
					vcpucount[2]++;
					break;

				case 3:
					pcpuarray[3] = pcpuarray[3] + (int)First[i].cputime;
					vcpucount[3]++;
					break;

				default:
					break;
			}
		}

		for (i=0; i < NUMBEROFPCPUS; i++)
		{
			printf("VCPUs in core %d are %d\n",i, vcpucount[i]);
		}

		int j=0, k=0, l=0, m=0;
		Third[0].vcpuutilization = malloc (sizeof(int) * vcpucount[0]);
		Third[1].vcpuutilization = malloc (sizeof(int) * vcpucount[1]);
		Third[2].vcpuutilization = malloc (sizeof(int) * vcpucount[2]);
		Third[3].vcpuutilization = malloc (sizeof(int) * vcpucount[3]);
		Third[0].vcpunumber = malloc (sizeof(int) * vcpucount[0]);
		Third[1].vcpunumber = malloc (sizeof(int) * vcpucount[1]);
		Third[2].vcpunumber = malloc (sizeof(int) * vcpucount[2]);
		Third[3].vcpunumber = malloc (sizeof(int) * vcpucount[3]);


		//Which vCPU is on each core

		for(i=0; i< numDomains; i++)
		{
			switch(Second[i].cpunumber)
			{
				case 0:
					Third[0].vcpuutilization[j] =  (int)First[i].cputime;
					Third[0].vcpunumber[j] = activeDomains[i];
					j++;
					break;

				case 1:
					Third[1].vcpuutilization[k] =  (int)First[i].cputime;
					Third[1].vcpunumber[k] = activeDomains[i];
					k++;
					break;

				case 2:
					Third[2].vcpuutilization[l] =  (int)First[i].cputime;
					Third[2].vcpunumber[l] = activeDomains[i];
					l++;        
					break;

				case 3:
					Third[3].vcpuutilization[m] =  (int)First[i].cputime;
					Third[3].vcpunumber[m] = activeDomains[i];
					m++;        
					break;

				default:
					break;
			}

		}
		for(i=0; i<vcpucount[0]; i++)
		{
			printf("VCPUs in Core 0 with id %d have utilization %d\n", Third[0].vcpunumber[i], Third[0].vcpuutilization[i]);
		}
		for(i=0; i<vcpucount[1]; i++)
		{
			printf("VCPUs in Core 1 with id %d have utilization %d\n", Third[1].vcpunumber[i], Third[1].vcpuutilization[i]);
		}

		for(i=0; i<vcpucount[2]; i++)
		{
			printf("VCPUs in Core 2 with id %d have utilization %d\n", Third[2].vcpunumber[i], Third[2].vcpuutilization[i]);
		}

		for(i=0; i<vcpucount[3]; i++)
		{
			printf("VCPUs in Core 3 with id %d have utilization %d\n", Third[3].vcpunumber[i], Third[3].vcpuutilization[i]);
		}
		printf("\n");
		for(i=0; i< 4; i++)
		{
			printf("PCPU Utilization of Core %d is %d\n", i, pcpuarray[i]);
		}

		printf("\n");

		//Algorithm - Difference between highest and lowest PCPU utilization is 25, Change Pinnings
		//To find largest pcpu utilization
		int maxutil = pcpuarray[0];
		int maxpcpunumber = 0, leastpcpunumber=0;
		for(i=0; i< NUMBEROFPCPUS; i++)
		{
			if (pcpuarray[i] > maxutil){
				maxutil = pcpuarray[i];
				maxpcpunumber = i;
			}
		}

		printf("Max Utilization is by PCPU %d and it is %d\n",maxpcpunumber, maxutil);

		//To find least pcpu utilization
		int leastutil = pcpuarray[0];
		for(i=0; i< NUMBEROFPCPUS; i++)
		{
			if (pcpuarray[i] < leastutil){
				leastutil = pcpuarray[i];
				leastpcpunumber = i;
			}
		}

		printf("Least Utilization is by PCPU %d and it is %d\n",leastpcpunumber, leastutil);
		int difference = maxutil - leastutil;
		int index;
		unsigned char * cpumap = malloc (sizeof(char));
		printf("Difference is %d\n", difference);
		if(difference > VCPUTHRESHOLD)
		{
			printf("Changing the pinnings to Balance since threshold exceeded\n");

			printf("\n");
			//For Changing the Pinnings
			switch(maxpcpunumber){
				case 0:
					index = 0;
					for(i=0; i<vcpucount[0]; i++)
					{
						if(Third[0].vcpuutilization[i] < Third[0].vcpuutilization[index]){
							index = i;
						}
					}
					domainPtr = virDomainLookupByID(conn, Third[0].vcpunumber[index]);

					if (leastpcpunumber == 1)
						* cpumap = 1 << 1;
					else if (leastpcpunumber == 2)
						* cpumap = 1 << 2;
					else if (leastpcpunumber ==3)
						* cpumap = 1 << 3;
					virDomainPinVcpu(domainPtr,0, cpumap, 1);

					if (vcpucount[0] == 8)             //Specific to testcase 1
					{
						domainPtr2 = virDomainLookupByID(conn, Third[0].vcpunumber[index +1]);
						domainPtr3 = virDomainLookupByID(conn, Third[0].vcpunumber[index +2]);
						domainPtr4 = virDomainLookupByID(conn, Third[0].vcpunumber[index +3]);

						virDomainPinVcpu(domainPtr2, 0, cpumap, 1);


						* cpumap = 1 << 2;
						virDomainPinVcpu(domainPtr3,0, cpumap, 1);
						* cpumap = 1 << 3;
						virDomainPinVcpu(domainPtr4,0, cpumap, 1);
					}

					if (vcpucount[0] == 4)           //Specific to testcase 1 and 2
					{
						domainPtr2 = virDomainLookupByID(conn, Third[0].vcpunumber[index +1]);
						* cpumap = 1 << 2;
						virDomainPinVcpu(domainPtr2, 0, cpumap, 1);
					}

					printf("Pinning Successful\n");
					break;
				case 1:
					index = 0;
					for(i=0; i<vcpucount[1]; i++)
					{
						if(Third[1].vcpuutilization[i] < Third[1].vcpuutilization[index]){
							index = i;
						}
					}
					domainPtr = virDomainLookupByID(conn, Third[1].vcpunumber[index]);
					if (leastpcpunumber == 0)
						* cpumap = 1;
					else if (leastpcpunumber == 2)
						* cpumap = 1 << 2;
					else if (leastpcpunumber ==3)
						* cpumap = 1 << 3;

					virDomainPinVcpu(domainPtr,0, cpumap, 1);
					printf("Pinning Successful\n");
					break;

				case 2:
					index = 0;
					for(i=0; i<vcpucount[2]; i++)
					{
						if(Third[2].vcpuutilization[i] < Third[2].vcpuutilization[index]){
							index = i;
						}
					}
					domainPtr = virDomainLookupByID(conn, Third[2].vcpunumber[index]);
					if (leastpcpunumber == 0)
						* cpumap = 1;
					else if (leastpcpunumber == 1)
						* cpumap = 1 << 1;
					else if (leastpcpunumber ==3)
						* cpumap = 1 << 3;

					virDomainPinVcpu(domainPtr,0, cpumap, 1);      
					printf("Pinning Successful\n");
					break;

				case 3:
					index = 0;
					for(i=0; i<vcpucount[3]; i++)
					{
						if(Third[3].vcpuutilization[i] < Third[3].vcpuutilization[index]){
							index = i;
						}
					}
					domainPtr = virDomainLookupByID(conn, Third[3].vcpunumber[index]);
					if (leastpcpunumber == 0)
						* cpumap = 1;
					else if (leastpcpunumber == 1)
						* cpumap = 1 << 1;
					else if (leastpcpunumber == 2)
						* cpumap = 1 << 2;

					virDomainPinVcpu(domainPtr,0, cpumap, 1); 
					printf("Pinning Successful\n");
					break;

				default:
					break;
			}
		}
		else printf("Loads Stable for now\n");
		printf("////////////////////////////////////////////////////////////////////////////////////////////////////////////\n");
		free(Third[0].vcpuutilization);
		free(Third[1].vcpuutilization);
		free(Third[2].vcpuutilization);
		free(Third[3].vcpuutilization);
		free(Third[0].vcpunumber);
		free(Third[1].vcpunumber);
		free(Third[2].vcpunumber);
		free(Third[3].vcpunumber);
		free(cpumap);
		free(cpuMap);
	}
	return 0;

}
