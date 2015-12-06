#!/usr/bin/python

import os, re, urllib2, platform, shutil, tarfile, zipfile
from sys import stdout


# Constants:
LINUX_HELP = """ -> your Linux seems to be %s-based. Please install the SDL and OpenEXR
    libraries. This can be done by, e.g., "sudo %s install %s %s"."""
MAC_HELP = """ -> use your package manager (MacPorts or Brew) to install the SDL and OpenEXR libraries."""
WIN_HELP = """
***
Setup is ready! Open `%s' with %s and start hacking!
***
"""

URL_SDL = { 
	"cb": "http://libsdl.org/release/SDL-devel-1.2.15-mingw32.tar.gz",
	"vs": "http://libsdl.org/release/SDL-devel-1.2.15-VC.zip"
}
URL_OPENEXR = {
	"cb": "http://raytracing-bg.net/lib/openexr-devel-mingw32.zip",
	"vs": "http://raytracing-bg.net/lib/openexr-devel-VC.zip",
}

OS = platform.uname()[0]


# Utility functions:
def copy_and_maybe_overwrite(source_file, target_file):
	print "Copying `%s' to `%s'..." % (source_file, target_file)
	if os.path.exists(target_file):
		print " -> target file exists, confirm overwrite\n    (local changes to it will be lost)", 
		choice = 'x'
		while choice.lower() not in ['y', 'n']:
			choice = raw_input("(y/n)? ")
		if choice.lower() == 'n':
			return
	shutil.copyfile(source_file, target_file)

def download_file(url, dest_dir):
	"""Fetches a file from a given URL and writes it to the given directory.
	   Returns: The path to the downloaded file.
	"""
	dest_file = os.path.join(dest_dir, url.split('/')[-1])
	if os.path.exists(dest_file):
		print "File `%s' is already downloaded." % dest_file
		return dest_file
	u = urllib2.urlopen(url)
	f = open(dest_file, 'wb')
	meta = u.info()
	file_size = int(meta.getheaders("Content-Length")[0])
	print "Downloading: %s (%d KiB)" % (dest_file, file_size / 1024)

	file_size_dl = 0
	block_sz = 65536
	while True:
		buffer = u.read(block_sz)
		if not buffer:
			break

		file_size_dl += len(buffer)
		f.write(buffer)
		status = "\r%10d KiB  [%5.1f%%]" % (file_size_dl / 1024, file_size_dl * 100. / file_size)
		stdout.write(status)
		stdout.flush()
	f.close()
	print "\rDone.                    "
	return dest_file

def mkdir_if_doesnt_exist(dirname):
	if not os.path.exists(dirname):
		os.mkdir(dirname)

def unpack_archive(archive, target_dir):
	print "Unpacking %s..." % archive, 
	arc = None
	if archive.endswith("tar.gz"):
		arc = tarfile.open(archive, "r")
	else:
		# assume .zip:
		arc = zipfile.ZipFile(archive, "r")
	arc.extractall(target_dir)
	arc.close()
	print "Done."

def setup_windows_SDKs(ide):
	mkdir_if_doesnt_exist("SDK")
	downloads_dir = os.path.join("SDK", "downloads")
	mkdir_if_doesnt_exist(downloads_dir)
	sdl_archive = download_file(URL_SDL[ide], downloads_dir)
	openexr_archive = download_file(URL_OPENEXR[ide], downloads_dir)

	unpack_archive(sdl_archive, "SDK")
	unpack_archive(openexr_archive, "SDK")

	if ide == "cb":
		copy_and_maybe_overwrite("qdamage-win32.cbp", "qdamage.cbp")
		print WIN_HELP % ("qdamage.cbp", "Code::Blocks")
	else:
		copy_and_maybe_overwrite("qdamage.sln.in", "qdamage.sln")
		copy_and_maybe_overwrite("qdamage.vcprojx.in", "qdamage.vcprojx")
		print WIN_HELP % ("qdamage.sln", "Visual C++")

	if not os.path.exists("SDL.dll"):
		shutil.copyfile(os.path.join("SDK", "SDL-1.2.15", "bin", "SDL.dll"), "SDL.dll")



def setup_environment_windows():
	print """What IDE would you like to use for development:
	1) Code::Blocks 13.12 or newer
	2) Microsoft Visual C++ 2013 or newer"""
	choice = "x"
	while choice not in ['1', '2']:
		choice = raw_input("? ")
	##
	setup_windows_SDKs("cb" if choice == '1' else "vs")

def setup_environment_unix():
	copy_and_maybe_overwrite("qdamage-linux.cbp", "qdamage.cbp")

def main():
	# change dir to the root of the repo:
	if os.path.exists("../qdamage-linux.cbp"):
		os.chdir("..")
	if not os.path.exists("qdamage-linux.cbp"):
		print "error: this script needs to be called from within the root repository dir, or from scripts/"
		return

	# determine OS:
	print "Detecting OS...", OS

	if OS == "Windows":
		setup_environment_windows()
	elif OS == "Linux":
		# On linux, do a crude detection of the distro lineage. We support Debian and RedHat derivates.
		if os.path.exists("/usr/bin/apt-get"):
			print LINUX_HELP % ("Debian", "apt-get", "libsdl-dev", "libopenexr-dev")
		elif os.path.exists("/usr/bin/yum"):
			print LINUX_HELP % ("Fedora", "yum", "SDL-devel", "OpenEXR-devel")
		else:
			print " -> your Linux distro is not recognized by this stupid tool. But you're a grown-up, you can handle this yourself!"
		setup_environment_unix()
	elif OS == "Darwin":
		print MAC_HELP
		setup_environment_unix()
	else:
		print " -> unknown OS, you are on your own (but feel free to send cheers if you get it running :))."


if __name__ == "__main__":
	main()
	if OS == "Windows":
		print "Press ENTER to close this script...",
		raw_input()
