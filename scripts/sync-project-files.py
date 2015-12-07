#!/usr/bin/python

"""A script to check if the list of .cpp/.h files in the *.cbp, *.vcxproj project files match up.

Usage: from within the main repo dir, call "scripts/sync-project-files.py".
"""

import os, re

source_files = {}

def analyze(project_file):
	global source_files
	if project_file.endswith(".cbp"):
		lineexp = re.compile(r'<Unit (filename)="src/([a-z0-9._]+)" />')
	elif project_file.find("vcxproj") != -1:
		lineexp = re.compile(r'<Cl(Compile|Include) Include="src\\([a-z0-9._]+)" />')
	
	if not os.path.exists(project_file):
		return
	
	f = open(project_file, "rt")
	for line in f:
		if lineexp.search(line):
			source_file = lineexp.findall(line)[0][1]
			if source_file not in source_files:
				source_files[source_file] = []
			source_files[source_file].append(project_file)
	f.close()

def format_diff(containing, expected):
	if len(containing) <= len(expected) / 2:
		return "only in %s" % ','.join(containing)
	else:
		return "missing in %s" % ','.join(filter(lambda s: s not in containing, expected))

def main():
	project_files = [
		"qdamage-linux.cbp",
		"qdamage-win32.cbp",
		"qdamage.vcxproj.in"] + filter(os.path.exists, ["qdamage.cbp", "qdamage.vcxproj"])

	for project_file in project_files:
		analyze(project_file)

	discrepancies = False
	for fn in source_files:
		if len(source_files[fn]) != len(project_files):
			if not discrepancies:
				discrepancies = True
				print "Discrepancies:"
			print "%-20s (%s)" % (fn, format_diff(source_files[fn], project_files))

	if not discrepancies:
		print "No discrepancies, all OK."

if __name__ == "__main__":
	main()
