#!/bin/bash -
# * ===============================================================================
# *   DESCRIPTION: This example script shows (1) how create inventory files
# *                listing dst files and (2) how to use those to create a job
# *                files for data analysis. It is desinged to illustrate the use
# *                of functions in the CommonFunctions.sh script.
# *        AUTHOR: Remco de Boer <remco.de.boer@ihep.ac.cn>
# *  ORGANIZATION: IHEP, CAS (Beijing, CHINA)
# *       CREATED: 23 November 2018
# *         USAGE: bash ExampleScript_CreateDataJoboptions.sh
# * ===============================================================================

source CommonFunctions.sh

# * Scripts parameters * #
packageName="D0phi_KpipiKK"
nFilesPerJob=100
nEventsPerJob=-1
outputLevel=4
data_or_MC=2 # 1: exclusive MC, 2: inclusive MC, 3: data

# * In case of analysing EXclusive Monte Carlo output * #
	if [ ${data_or_MC} == 1 ]; then 
		directoryToRead="/scratchfs/bes/deboer/data/dst/${packageName}/100,000_events"
		identifier="${packageName}_excl"
# * In case of analysing INclusive Monte Carlo output * #
	elif [ ${data_or_MC} == 2 ]; then
		fileToRead="directories/incl/incl_Jpsi2009"
		nFilesPerJob=100
		identifier="${packageName}_incl"
# * In case of analysing real BESIII data * #
	elif [ ${data_or_MC} == 3 ]; then
		fileToRead="directories/data/data_Jpsi2018_round11"
		nFilesPerJob=100
		identifier="${packageName}_data"
# * If not defined properly * #
	else
		echo "Option \"data_or_MC = ${data_or_MC}\" cannot be handled"
		exit 1
	fi

# * Set output subdirectory * #
	if [ "${directoryToRead}" != "" ]; then
		outputSubdir="${packageName}/$(basename ${directoryToRead})"
	fi
	if [ "${fileToRead}" != "" ]; then
		outputSubdir="${packageName}/$(basename ${fileToRead})"
	fi

# * Create job from template and submit * #
if [ "${fileToRead}" != "" ]; then
	# * This will create your job files based on a file listing dst files and directories
	CreateFilenameInventoryFromFile "${fileToRead}" "filenames/${identifier}_fromfile.txt" ${nFilesPerJob} "dst"
	bash CreateJobFiles_ana.sh "${packageName}" "filenames/${identifier}_fromfile_???.txt" ${nEventsPerJob} ${outputLevel} "${outputSubdir}"
else
	# * This will create your job files based on a directory containing dst files
	CreateFilenameInventoryFromDirectory "${directoryToRead}" "filenames/${identifier}.txt" ${nFilesPerJob} "dst"
	bash CreateJobFiles_ana.sh "${packageName}" "filenames/${identifier}_???.txt" ${nEventsPerJob} ${outputLevel} "${outputSubdir}"
fi

# * SUBMIT * #
bash SubmitAll.sh "${outputSubdir}_ana" "${packageName}_ana"