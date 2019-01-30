//CS 6210 AOS memory_coordinator
//Akshaya Nagarajan 
//GTID 903319262

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<libvirt/libvirt.h>
#define CONVERSION 1024
#define PERCENTAGE 0.35
#define UPPERUNUSEDTHRESHOLD 270
#define LOWERUNUSEDTHRESHOLD 100
#define HOSTMEMORYTHRESHOLD 100 
#define LOWERBOUNDFORUNUSED 60

struct DomainMemoryStats{
	int cpuid;
	unsigned long long avl;
	unsigned long long unu;
	unsigned long long act;
};

struct HostMemoryStats{
	unsigned long long total;
	unsigned long long free;
};


struct HostMemoryStats HostMemoryStatistics(virConnectPtr conn){

	struct HostMemoryStats HoMemStats;
	int i;

	virNodeMemoryStatsPtr params = malloc(sizeof(virNodeMemoryStats));
	int nparams = 0;
	if (virNodeGetMemoryStats(conn, VIR_NODE_MEMORY_STATS_ALL_CELLS, NULL, &nparams, 0) == 0 && nparams != 0) {
		params = malloc(sizeof(virNodeMemoryStats) * nparams);
		memset(params, 0, sizeof(virNodeMemoryStats) * nparams);
		virNodeGetMemoryStats(conn, VIR_NODE_MEMORY_STATS_ALL_CELLS, params, &nparams, 0);
	}

	for (i=0; i< nparams; i++)
	{
		if(strcmp(params[i].field, "total") ==0){
			HoMemStats.total = ((params[i].value)/CONVERSION);

		}
		if(strcmp(params[i].field, "free") ==0){
			HoMemStats.free = ((params[i].value)/CONVERSION);


		}
	}

	return HoMemStats;
}

int main(int argc, char *argv[])
{
	virConnectPtr conn;
	int numDomains,i,j;
	int *activeDomains;
	int Startflag = 0;
	virDomainPtr domainPtr;

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
	activeDomains = malloc(sizeof(int) * numDomains);
	numDomains = virConnectListDomains(conn, activeDomains, numDomains);
	virDomainMemoryStatPtr stats;
	stats = malloc(sizeof(virDomainMemoryStatStruct) * VIR_DOMAIN_MEMORY_STAT_NR);
	printf("VirDomain Stats thingy %d\n", VIR_DOMAIN_MEMORY_STAT_NR);

	struct DomainMemoryStats DomMemStats[numDomains];
	struct HostMemoryStats HostMemStats;

	while(1){

		//Getting Host Memory Stats

		HostMemStats = HostMemoryStatistics(conn);
		printf("Host Total Memory is %llu\n", HostMemStats.total);
		printf("Host Free Memory is %llu\n", HostMemStats.free);



		printf("\n");

		//Getting VM memory Stats

		for (i = 0 ; i < numDomains ; i++) {
			printf("Domain ids are  %d\t", activeDomains[i]);
			domainPtr = virDomainLookupByID(conn, activeDomains[i]);
			DomMemStats[i].cpuid = activeDomains[i];
			printf("cpuid is %d\t", DomMemStats[i].cpuid);
			int Result = virDomainSetMemoryStatsPeriod(domainPtr, atoi(argv[1]),  VIR_DOMAIN_AFFECT_CURRENT);
			printf("Result is %d\t", Result);
			virDomainMemoryStats(domainPtr, stats, VIR_DOMAIN_MEMORY_STAT_NR, 0);
			for (j = 0; j< VIR_DOMAIN_MEMORY_STAT_NR; j++)
			{
				if(stats[j].tag == VIR_DOMAIN_MEMORY_STAT_AVAILABLE)
				{
					printf("Available tag %d\t", stats[j].tag);
					DomMemStats[i].avl = ((stats[j].val)/CONVERSION);
					printf("Available value %llu\t", DomMemStats[i].avl);
				}

				if(stats[j].tag == VIR_DOMAIN_MEMORY_STAT_ACTUAL_BALLOON)
				{
					printf("Actual tag %d\t", stats[j].tag);
					DomMemStats[i].act = ((stats[j].val)/CONVERSION);
					printf("Actual value %llu\t", DomMemStats[i].act);
				}

				if(stats[j].tag == VIR_DOMAIN_MEMORY_STAT_UNUSED)
				{
					printf("Unused tag %d\t", stats[j].tag);
					DomMemStats[i].unu = ((stats[j].val)/CONVERSION);
					printf("Unused value %llu\n", DomMemStats[i].unu);
				}

			}
			printf("\n");
			printf("\n");

			//Memory Allocation

			if (DomMemStats[i].act <= 550)
			{

				if ( (DomMemStats[i].unu < LOWERUNUSEDTHRESHOLD && DomMemStats[i].unu > LOWERBOUNDFORUNUSED) )
				{

					HostMemStats = HostMemoryStatistics(conn);
					printf("Host Free Memory is %llu\n", HostMemStats.free);

					if (HostMemStats.free > HOSTMEMORYTHRESHOLD) {
						printf("Setting the Memory\n");

						virDomainSetMemory(domainPtr, CONVERSION*((DomMemStats[i].act - DomMemStats[i].unu) + PERCENTAGE*(DomMemStats[i].act - DomMemStats[i].unu)));
						printf("Memory is Set\n");				  
					}
					else printf("Do Nothing\n");

				}

			}

			//Memory Deallocation

			if (Startflag > 3)
			{
				if ((DomMemStats[i].unu > UPPERUNUSEDTHRESHOLD))
				{
					printf("Setting the Memory\n");
					virDomainSetMemory(domainPtr, CONVERSION*((DomMemStats[i].act - DomMemStats[i].unu) + PERCENTAGE*(DomMemStats[i].act - DomMemStats[i].unu)));
					printf("Memory is Set\n");
				}
			}


		}
		Startflag++;
		printf("The Startflag count is %d\n", Startflag);
		printf("\n");


		printf("///////////////////////////////////////////////////////////////////////////////////\n");
		sleep(atoi(argv[1]));

	}
	return 0;
}
