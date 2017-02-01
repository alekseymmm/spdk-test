/*
 * main.c
 *
 *  Created on: 31 янв. 2017 г.
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rte_config.h>
#include <rte_eal.h>

#include "spdk/nvme.h"
#include "spdk/env.h"

struct ctrlr_entry{
	struct spdk_nvme_ctrlr	*ctrlr;
	struct ctrlr_entry 		*next;
	char					name[1024];
};

struct ns_entry {
	struct spdk_nvme_ctrlr  *ctrlr;
	struct spdk_nvme_ns		*ns;
	struct ns_entry			*next;
	struct spdk_nvme_qpair	*qpair;
};

static struct ctrlr_entry *g_controllers = NULL;
static struct ns_entry *g_namespaces = NULL;

static void register_ns(struct spdk_nvme_ctrlr *ctrlr, struct spdk_nvme_ns *ns){
	struct ns_entry *entry;
	const struct spdk_nvme_ctrlr_data *cdata;

	cdata = spdk_nvme_ctrlr_get_data(ctrlr);

	if(!spdk_nvme_ns_is_active(ns)){
		printf("Controller %-20.20s (%-20.20s): Skipping inactive NS %u\n",
				cdata->mn, cdata->sn, spdk_nvme_ns_get_id(ns));
		return;
	}

	entry = malloc(sizeof(struct ns_entry));
	if(entry == NULL){
		perror("ns_entry malloc");
		exit(1);
	}

	entry->ctrlr = ctrlr;
	entry->ns = ns;
	entry->next = g_namespaces;
	g_namespaces = entry;

	printf("Namespace ID: %d size: %ju GB\n", spdk_nvme_ns_get_id(ns),
			spdk_nvme_ns_get_size(ns) / 1000000000);
}

static bool probe_cb(void *cb_ctx, const struct spdk_nvme_transport_id *tid,
		struct spdk_nvme_ctrlr_opts * opts){

	printf("Attaching to %s\n", tid->traddr);

	return true;
}

static void attach_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
		struct spdk_nvme_ctrlr *ctrlr, const struct spdk_nvme_ctrlr_opts *opts){

	int nsid, num_ns;
	struct ctrlr_entry *entry;
	const struct spdk_nvme_ctrlr_data *cdata = spdk_nvme_ctrlr_get_data(ctrlr);

	entry = malloc(sizeof(struct ctrlr_entry));
	if (entry == NULL){
		perror("ctrlr_entry malloc failed");
		exit(1);
	}

	printf("Attached to %s\n", trid->traddr);

	snprintf(entry->name, sizeof(entry->name), "%-20.20s (%-20.20s)", cdata->mn, cdata->sn);

	entry->ctrlr = ctrlr;
	entry->next = g_controllers;
	g_controllers = entry;

	//note that in nmve namespace ID start at 1 not 0
	num_ns = spdk_nvme_ctrlr_get_num_ns(ctrlr);
	printf("Using controller %s with %d namespaces. \n", entry->name, num_ns);

	for(nsid = 1; nsid <= num_ns; nsid++){
		register_ns(ctrlr, spdk_nvme_ctrlr_get_ns(ctrlr, nsid));
	}

}

static char *ealargs[] = {
		"hello world",
		"-c 0x1",
		"-n 4",
		"--proc-type=auto",
};

static void cleanup(void){
	struct ns_entry *ns_entry = g_namespaces;
	struct ctrlr_entry *ctrlr_entry = g_controllers;

	while(ns_entry){
		struct ns_entry *next = ns_entry->next;
		free(ns_entry);
		ns_entry = next;
	}

	while(ctrlr_entry){
		struct ctrlr_entry *next =ctrlr_entry->next;

		spdk_nvme_detach(ctrlr_entry->ctrlr);
		free(ctrlr_entry);
		ctrlr_entry = next;
	}
}

int main(int argc, char **argv){
	int rc;

	rc = rte_eal_init(sizeof(ealargs) / sizeof(ealargs[0]), ealargs);
	if(rc < 0){
		fprintf(stderr, "Could not initialize dpdk\n");
		return 1;
	}

	printf("Initializing NVME controllers\n");

	/* Start nvme enumeration process. probe_cb will be called
	 * for each nvme controller found . attach_cb  will be called
	 * for each controller after SPDK driver has completed init
	 */
	rc = spdk_nvme_probe(NULL, NULL, probe_cb, attach_cb, NULL);
	if(rc != 0){
		fprintf(stderr, "spdk_nvme_probe() failed\n");
		cleanup();
		return 1;
	}

	printf("Initialization complete.\n");

	//hello_world();
	cleanup();
	return 0;
}
