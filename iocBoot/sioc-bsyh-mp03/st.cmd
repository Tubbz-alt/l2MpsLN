#!../../bin/linuxRT-x86_64/l2MpsLN
#
# SIOC:BSYH:MP03
#

< envPaths

# ===========================================
#            ENVIRONMENT VARIABLES
# ===========================================

#
# Link Node Configuration (generated from MPS database)
#
# This defines the location of the configuration files,
# we need know the cpu name, crate ID and slot number to 
# find the correct location.
#
epicsEnvSet("CONFIG_TOP", "/afs/slac/u/cd/lpiccoli/lcls2/mps_configuration/cu/link_node_db/app_db")
epicsEnvSet("CPU_NAME", "cpu-bsyh-sp02")
epicsEnvSet("CRATE_ID", "0000")
epicsEnvSet("SLOT_ID", "2")
epicsEnvSet("LN_CONFIG_TOP", "${CONFIG_TOP}/${CPU_NAME}/${CRATE_ID}/0${SLOT_ID}")
epicsEnvSet("SLOT_ID", "7") # This is later used to set the LN card IP address (testing it in slot 7)


#
# Loads generated mps environment variables for this SIOC, variables:
#
< ${LN_CONFIG_TOP}/mps.env

#
# Loads common Link Node startup
#
< ${TOP}/iocBoot/common/link_node.cmd
