import json
import hashlib
import subprocess
import shutil
import os

hashers = {
	"MD5":      hashlib.md5,
	"SHA-1":    hashlib.sha1,
	"SHA-224":  hashlib.sha224,
	"SHA-256":  hashlib.sha256,
	"SHA-384":  hashlib.sha384,
	"SHA-512":  hashlib.sha512,
	"BLAKE2b":  hashlib.blake2b,
	"BLAKE2s":  hashlib.blake2s,
	"SHA3-224": hashlib.sha3_224,
	"SHA3-256": hashlib.sha3_256,
	"SHA3-384": hashlib.sha3_384,
	"SHA3-512": hashlib.sha3_512,
	"SHAKE128": hashlib.shake_128,
	"SHAKE256": hashlib.shake_256,
}

class BuildException(Exception):
	pass

class RomVersion:
	def __init__(self, name, file, hashes, clean_roms_folder):
		self.name   = name
		self.file   = file
		self.path   = os.path.join(clean_roms_folder, file)
		self.hashes = hashes

	def verify_hash(self, hash_name):
		curr_hash = hashers[hash_name]()

		with open(self.path, 'rb') as file:
			for byte_block in iter(lambda: file.read(0x1000), b""):
				curr_hash.update(byte_block)

		hash_string = curr_hash.hexdigest()

		if hash_string == self.hashes[hash_name]:
			padded_name = hash_name.ljust(max(len(name) for name in hashers))
			print(f"{padded_name} verified for '{self.file}'")
			return

		raise BuildException(
			f"'{self.file}' has an incorrect {hash_name}:"
			f"\n\tThe {hash_name} of the ROM was {hash_string}"
			f"\n\twhile it should be {' ' * len(hash_name)} {self.hashes[hash_name]}"
		)

	def verify_hashes(self):
		for hash_name in self.hashes.keys():
			self.verify_hash(hash_name)

class ReleaseBuilder:
	def __init__(self, json_filename):
		with open(json_filename) as settings_file:
			parsed_json = json.load(settings_file)

		self.title              = parsed_json["title"]
		self.patch_folder_name  = parsed_json["patch_folder_name"]
		self.xdelta_folder_name = parsed_json["xdelta_folder_name"]
		self.rom_path           = parsed_json["rom_path"]
		self.clean_roms_folder  = parsed_json["clean_roms_folder"]
		self.include_folder     = parsed_json["include_folder"]
		self.xdelta_folder      = parsed_json["xdelta_folder"]
		self.release_folder     = parsed_json["release_folder"]

		self.release_name = f"{self.title} v{parsed_json['version']}"
		self.build_folder = os.path.join(self.release_folder, "build", self.release_name)
		self.deposit_folder = os.path.join(self.build_folder, self.patch_folder_name)
		self.test_folder = os.path.join(self.release_folder, "test")

		self.rom_versions = [
			RomVersion(**rom_version, clean_roms_folder=self.clean_roms_folder)
			for rom_version in parsed_json["rom_versions"]
		]

	def create_patch(self, rom_version):
		patch_path = os.path.join(self.deposit_folder, f"{self.title} ({rom_version.name}).xdelta")

		args = [
			os.path.join(self.xdelta_folder, "xdelta.exe"),
			"-f",
			"-s",
			rom_version.path,
			self.rom_path,
			patch_path
		]

		subprocess.run(args)
		print(f"Created patch '{patch_path}'")

	def apply_patch(self, rom_version, patch_folder, result_folder):
		patch_path = os.path.join(patch_folder, f"{self.title} ({rom_version.name}).xdelta")
		result_path = os.path.join(result_folder, f"{self.title} ({rom_version.name}).nds")

		args = [
			os.path.join(self.xdelta_folder, "xdelta.exe"),
			"-d",
			"-f",
			"-s",
			rom_version.path,
			patch_path,
			result_path
		]

		subprocess.run(args)
		print(f"Created patched ROM '{result_path}'")
		return result_path

	def remove_release_folder(self):
		if shutil.os.path.exists(self.release_folder) and shutil.os.path.isdir(self.release_folder):
			shutil.rmtree(self.release_folder)
			print(f"Removed folder '{self.release_folder}'")

	def create_folders(self):
		os.makedirs(self.release_folder)
		print(f"Created folder '{self.release_folder}'")

		os.makedirs(self.build_folder)
		print(f"Created folder '{self.build_folder}'")

		os.makedirs(self.deposit_folder)
		print(f"Created folder '{self.deposit_folder}'")

	def copy_includes(self):
		for filename in os.listdir(self.include_folder):
			source_path = os.path.join(self.include_folder, filename)
			dest_path   = os.path.join(self.build_folder, filename)

			if shutil.os.path.isdir(source_path):
				shutil.copytree(source_path, dest_path)
			else:
				shutil.copy(source_path, dest_path)

			print(f"Copied '{source_path}' to '{self.build_folder}'")

	def copy_xdelta(self):
		xdelta_dest_path = os.path.join(self.build_folder, self.xdelta_folder_name)
		shutil.copytree(self.xdelta_folder, xdelta_dest_path)
		print(f"Copied folder '{self.xdelta_folder}' to '{xdelta_dest_path}'")

	def copy_license(self):
		shutil.copy("LICENSE", self.build_folder)
		print(f"Copied LICENSE")

	def make_zip(self):
		zip_path = shutil.make_archive(
			os.path.join(self.release_folder, self.release_name),
			"zip",
			os.path.join(self.release_folder, "build")
		)

		print(f"Created zip file '{zip_path}'")
		return zip_path

	def test_patches(self, zip_path):
		print("Testing patches...")
		os.makedirs(self.test_folder)
		print(f"Created folder '{self.test_folder}'")

		shutil.unpack_archive(zip_path, self.test_folder, "zip")
		print(f"Extracted the archive to '{self.test_folder}")
		patch_folder = os.path.join(self.test_folder, self.release_name, self.patch_folder_name)

		patched_rom_paths = []
		for rom in self.rom_versions:
			patched_rom_paths.append(self.apply_patch(rom, patch_folder, self.test_folder))

		with open(self.rom_path, 'rb') as correct_rom_file:
			correct_rom = list(correct_rom_file.read())

		for patched_rom_path in patched_rom_paths:
			with open(patched_rom_path, 'rb') as patched_rom_file:
				patched_rom = list(patched_rom_file.read())

				if patched_rom == correct_rom:
					print(f"Patched ROM '{patched_rom_path}' matches with '{self.rom_path}'")
				else:
					raise BuildException(f"Patched ROM '{patched_rom_path}' doesn't match with '{self.rom_path}'")

	def handle_error(self, exception):
		print(f"ERROR: {exception}")
		self.remove_release_folder()
		print("Build aborted")

	def build(self):
		self.remove_release_folder()

		try:
			for rom in self.rom_versions:
				rom.verify_hashes()
		except BuildException as e:
			self.handle_error(e)
			return

		self.create_folders()

		for rom in self.rom_versions:
			self.create_patch(rom)

		self.copy_xdelta()
		self.copy_includes()
		self.copy_license()
		zip_path = self.make_zip()

		try:
			self.test_patches(zip_path)
		except BuildException as e:
			self.handle_error(e)
			return

		print("Release created successfully")

if __name__ == "__main__":
	ReleaseBuilder("release_settings.json").build()
