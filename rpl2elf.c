// Copyright (C) 2015       Hykem <hykem@hotmail.com>
// Licensed under the terms of the GNU GPL, version 3
// http://www.gnu.org/licenses/gpl-3.0.txt

#include "rpl2elf.h"

// Endian swap file write functions.
void fwrite16(u16 i, FILE *f)
{
	u16 p = se16(i);
	fwrite(&p, sizeof(u16), 1, f);
}

void fwrite32(u32 i, FILE *f)
{
	u32 p = se32(i);
	fwrite(&p, sizeof(u32), 1, f);
}

void fwrite64(u64 i, FILE *f)
{
	u64 p = se64(i);
	fwrite(&p, sizeof(u64), 1, f);
}

// Print functions.
void print_elf32_ehdr(Elf32_Ehdr *ehdr)
{
	printf("ELF header:\n");
	printf("e_ident     %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
			ehdr->e_ident[0], ehdr->e_ident[1], ehdr->e_ident[2], ehdr->e_ident[3],
			ehdr->e_ident[4], ehdr->e_ident[5], ehdr->e_ident[6], ehdr->e_ident[7],
			ehdr->e_ident[8], ehdr->e_ident[9], ehdr->e_ident[10], ehdr->e_ident[11],
			ehdr->e_ident[12], ehdr->e_ident[13], ehdr->e_ident[14], ehdr->e_ident[15]);
	printf("e_type      0x%04x [%s]\n", ehdr->e_type, (ehdr->e_type == 0xFE01) ? "RPL" : "UNKNOWN");
	printf("e_machine   0x%04x [%s]\n", ehdr->e_machine, (ehdr->e_machine == 0x0014) ? "PowerPC" : "UNKNOWN");
	printf("e_version   0x%08x\n", ehdr->e_version);
	printf("e_entry     0x%08x\n", ehdr->e_entry);
	printf("e_phoff     0x%08x\n", ehdr->e_phoff);
	printf("e_shoff     0x%08x\n", ehdr->e_shoff);
	printf("e_flags     0x%08x\n", ehdr->e_flags);
	printf("e_ehsize    0x%04x\n", ehdr->e_ehsize);
	printf("e_phentsize 0x%04x\n", ehdr->e_phentsize);
	printf("e_phnum     0x%04x\n", ehdr->e_phnum);
	printf("e_shentsize 0x%04x\n", ehdr->e_shentsize);
	printf("e_shnum     0x%04x\n", ehdr->e_shnum);
	printf("e_shstrndx  0x%04x\n", ehdr->e_shstrndx);
	printf("\n");
}

void print_elf32_shdr(Elf32_Shdr *shdr, int i)
{
	printf("ELF section header #%d:\n", i);
	printf("sh_name:       0x%08x\n", shdr->sh_name);
	printf("sh_type:       0x%08x\n", shdr->sh_type);
	printf("sh_flags:      0x%08x\n", shdr->sh_flags);
	printf("sh_addr:       0x%08x\n", shdr->sh_addr);
	printf("sh_offset:     0x%08x\n", shdr->sh_offset);
	printf("sh_size:       0x%08x\n", shdr->sh_size);
	printf("sh_link:       0x%08x\n", shdr->sh_link);
	printf("sh_info:       0x%08x\n", shdr->sh_info);
	printf("sh_addralign:  0x%08x\n", shdr->sh_addralign);
	printf("sh_entsize:    0x%08x\n", shdr->sh_entsize);
	printf("\n");
}

void print_rpl_fileinfo(Rpl_Fileinfo *fileinfo)
{
	printf("RPL file info:\n");
	printf("magic_version:                  0x%08x\n", fileinfo->magic_version);
	printf("mRegBytes_Text:                 0x%08x\n", fileinfo->mRegBytes_Text);
	printf("mRegBytes_TextAlign:            0x%08x\n", fileinfo->mRegBytes_TextAlign);
	printf("mRegBytes_Data:                 0x%08x\n", fileinfo->mRegBytes_Data);
	printf("mRegBytes_DataAlign:            0x%08x\n", fileinfo->mRegBytes_DataAlign);
	printf("mRegBytes_LoaderInfo:           0x%08x\n", fileinfo->mRegBytes_LoaderInfo);
	printf("mRegBytes_LoaderInfoAlign:      0x%08x\n", fileinfo->mRegBytes_LoaderInfoAlign);
	printf("mRegBytes_Temp:                 0x%08x\n", fileinfo->mRegBytes_Temp);
	printf("mTrampAdj:                      0x%08x\n", fileinfo->mTrampAdj);
	printf("mSDABase:                       0x%08x\n", fileinfo->mSDABase);
	printf("mSDA2Base:                      0x%08x\n", fileinfo->mSDA2Base);
	printf("mSizeCoreStacks:                0x%08x\n", fileinfo->mSizeCoreStacks);
	printf("mSrcFileNameOffset:             0x%08x\n", fileinfo->mSrcFileNameOffset);
	printf("mFlags:                         0x%08x\n", fileinfo->mFlags);
	printf("mSysHeapBytes:                  0x%08x\n", fileinfo->mSysHeapBytes);
	printf("mTagsOffset:                    0x%08x\n", fileinfo->mTagsOffset);
	printf("\n");
}

void print_rpl_crcs(uint32_t *crcs, uint32_t crcs_size)
{
	printf("RPL data CRCs:\n");
	int i;
	for (i = 0; i < (crcs_size / 0x4); i++)
	{
		if (((i % 4) == 0) && (i > 0))
			printf("\n");
		printf("0x%08x  ", crcs[i]);
	}
	printf("\n");
}

// RPL functions.
int convert_rpl(FILE *in, FILE *out)
{
	// Get the input file's size.
	fseek(in, 0, SEEK_END);
	int file_size = ftell(in);
	fseek(in, 0, SEEK_SET);
	
	// Read the input RPL file.
	uint8_t *rpl = (uint8_t *) malloc (file_size);
	memset(rpl, 0, file_size);
	fread(rpl, file_size, 1, in);
	
	// Prepare to read the RPL's ELF EHDR.
	uint32_t ehdr_size = sizeof(Elf32_Ehdr);
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *) malloc (ehdr_size);
	memset(ehdr, 0, ehdr_size);

	// Read ELF EHDR.
	int i;
	for(i = 0; i < 0x10; i++)
		ehdr->e_ident[i] = rpl[i];
	ehdr->e_type    	 = be16(rpl + 0x10);
	ehdr->e_machine 	 = be16(rpl + 0x12);
	ehdr->e_version 	 = be32(rpl + 0x14);
	ehdr->e_entry   	 = be32(rpl + 0x18);
	ehdr->e_phoff   	 = be32(rpl + 0x1c);
	ehdr->e_shoff   	 = be32(rpl + 0x20);
	ehdr->e_flags   	 = be32(rpl + 0x24);
	ehdr->e_ehsize  	 = be16(rpl + 0x28);
	ehdr->e_phentsize 	 = be16(rpl + 0x2a);
	ehdr->e_phnum     	 = be16(rpl + 0x2c);
	ehdr->e_shentsize 	 = be16(rpl + 0x2e);
	ehdr->e_shnum     	 = be16(rpl + 0x30);
	ehdr->e_shstrndx  	 = be16(rpl + 0x32);

	// Print ELF EHDR.
	print_elf32_ehdr(ehdr);

	// Allocate memory for the special RPL file info section.
	uint32_t fileinfo_size = sizeof(Rpl_Fileinfo);
	Rpl_Fileinfo *fileinfo = (Rpl_Fileinfo *) malloc (fileinfo_size);
	memset(fileinfo, 0, fileinfo_size);
	
	// Allocate memory for the SHDR entries' CRCs.
	uint32_t crcs_size = sizeof(uint32_t) * ehdr->e_shnum;
	uint32_t *crcs = (uint32_t *) malloc (crcs_size);
	memset(crcs, 0, crcs_size);
	
	// Prepare to read the RPL's ELF section table.
	uint32_t shdr_table_size = ehdr->e_shnum * sizeof(Elf32_Shdr *);
	Elf32_Shdr** shdr_table = (Elf32_Shdr**) malloc (shdr_table_size);
	memset(shdr_table, 0, shdr_table_size);
	
	// Set SHDR offsets.
	uint32_t shdr_rpl_offset = ehdr->e_shoff;
	uint32_t shdr_elf_offset = ehdr->e_ehsize;
	uint32_t shdr_data_elf_offset = shdr_elf_offset + ehdr->e_shnum * ehdr->e_shentsize;
	
	// Read each ELF SHDR entry.
	for (i = 0; i < ehdr->e_shnum; i++)
	{
		// Allocate memory for each entry.
		shdr_table[i] = (Elf32_Shdr *) malloc (sizeof(Elf32_Shdr));
		memset(shdr_table[i], 0, sizeof(Elf32_Shdr));
		
		// Read entry.
		shdr_table[i]->sh_name     	 = be32(rpl + shdr_rpl_offset + 0x00);
		shdr_table[i]->sh_type 	   	 = be32(rpl + shdr_rpl_offset + 0x04);
		shdr_table[i]->sh_flags    	 = be32(rpl + shdr_rpl_offset + 0x08);
		shdr_table[i]->sh_addr     	 = be32(rpl + shdr_rpl_offset + 0x0c);
		shdr_table[i]->sh_offset   	 = be32(rpl + shdr_rpl_offset + 0x10);
		shdr_table[i]->sh_size     	 = be32(rpl + shdr_rpl_offset + 0x14);
		shdr_table[i]->sh_link     	 = be32(rpl + shdr_rpl_offset + 0x18);
		shdr_table[i]->sh_info     	 = be32(rpl + shdr_rpl_offset + 0x1c);
		shdr_table[i]->sh_addralign  = be32(rpl + shdr_rpl_offset + 0x20);
		shdr_table[i]->sh_entsize  	 = be32(rpl + shdr_rpl_offset + 0x24);
				
		// Check for valid section offset.
		if (shdr_table[i]->sh_offset != 0)
		{
			// Read the special RPL file info section.
			if ((shdr_table[i]->sh_type & SHT_RPL_FILEINFO) == SHT_RPL_FILEINFO)
			{
				fileinfo->magic_version					= be32(rpl + shdr_table[i]->sh_offset + 0x00);
				fileinfo->mRegBytes_Text				= be32(rpl + shdr_table[i]->sh_offset + 0x04);
				fileinfo->mRegBytes_TextAlign			= be32(rpl + shdr_table[i]->sh_offset + 0x08);
				fileinfo->mRegBytes_Data				= be32(rpl + shdr_table[i]->sh_offset + 0x0c);
				fileinfo->mRegBytes_DataAlign			= be32(rpl + shdr_table[i]->sh_offset + 0x10);
				fileinfo->mRegBytes_LoaderInfo			= be32(rpl + shdr_table[i]->sh_offset + 0x14);
				fileinfo->mRegBytes_LoaderInfoAlign		= be32(rpl + shdr_table[i]->sh_offset + 0x18);
				fileinfo->mRegBytes_Temp				= be32(rpl + shdr_table[i]->sh_offset + 0x1c);
				fileinfo->mTrampAdj						= be32(rpl + shdr_table[i]->sh_offset + 0x20);
				fileinfo->mSDABase						= be32(rpl + shdr_table[i]->sh_offset + 0x24);
				fileinfo->mSDA2Base						= be32(rpl + shdr_table[i]->sh_offset + 0x28);
				fileinfo->mSizeCoreStacks				= be32(rpl + shdr_table[i]->sh_offset + 0x2c);
				fileinfo->mSrcFileNameOffset			= be32(rpl + shdr_table[i]->sh_offset + 0x30);
				fileinfo->mFlags						= be32(rpl + shdr_table[i]->sh_offset + 0x34);
				fileinfo->mSysHeapBytes					= be32(rpl + shdr_table[i]->sh_offset + 0x38);
				fileinfo->mTagsOffset					= be32(rpl + shdr_table[i]->sh_offset + 0x3c);
			}
			else if ((shdr_table[i]->sh_type & SHT_RPL_CRCS) == SHT_RPL_CRCS)
			{
				// Read the SHDR entries' CRC table.
				memcpy(crcs, rpl + shdr_table[i]->sh_offset, crcs_size);
			}
			
			// Read each SHDR entry's data.
			if ((shdr_table[i]->sh_type & SHT_NOBITS) != SHT_NOBITS)
			{
				// Decompress the data if necessary.
				if ((shdr_table[i]->sh_flags & SHF_RPL_ZLIB) == SHF_RPL_ZLIB)
				{
					// Decompress the SHDR data.
					uint32_t shdr_data_size = be32(rpl + shdr_table[i]->sh_offset + 0x00);
					uint8_t *shdr_data = (uint8_t *) malloc (shdr_data_size);
					inflate_data(rpl + shdr_table[i]->sh_offset + 0x04, shdr_table[i]->sh_size, shdr_data, shdr_data_size);
					
					// Write the SHDR data to the output file.
					fseek(out, shdr_data_elf_offset, SEEK_SET);
					fwrite(shdr_data, shdr_data_size, 1, out);
					free(shdr_data);
					
					// Fix the SHDR entry's flags and size.
					shdr_table[i]->sh_flags &= ~SHF_RPL_ZLIB;
					shdr_table[i]->sh_size = shdr_data_size;
				}
				else
				{
					// Write the SHDR data to the output file.
					fseek(out, shdr_data_elf_offset, SEEK_SET);
					fwrite(rpl + shdr_table[i]->sh_offset, shdr_table[i]->sh_size, 1, out);
				}		
				
				// Increment the data offset.
				shdr_table[i]->sh_offset = shdr_data_elf_offset;
				shdr_data_elf_offset += shdr_table[i]->sh_size;
			}
		}
		
		// Write the modified ELF SHDR to the output file.
		fseek(out, shdr_elf_offset, SEEK_SET);
		fwrite32(shdr_table[i]->sh_name, out);
		fwrite32(shdr_table[i]->sh_type, out);
		fwrite32(shdr_table[i]->sh_flags, out);
		fwrite32(shdr_table[i]->sh_addr, out);
		fwrite32(shdr_table[i]->sh_offset, out);
		fwrite32(shdr_table[i]->sh_size, out);
		fwrite32(shdr_table[i]->sh_link, out);
		fwrite32(shdr_table[i]->sh_info, out);
		fwrite32(shdr_table[i]->sh_addralign, out);
		fwrite32(shdr_table[i]->sh_entsize, out);
		
		// Increment the SHDR entries' offsets.
		shdr_rpl_offset += ehdr->e_shentsize;
		shdr_elf_offset += sizeof(Elf32_Shdr);
		
		// Print ELF SHDR.
		print_elf32_shdr(shdr_table[i], i);
	}
	
	// Print RPL fileinfo.
	print_rpl_fileinfo(fileinfo);
				
	// Print the CRC table.
	print_rpl_crcs(crcs, crcs_size);
	
	// Switch ELF type to ET_EXEC.
	ehdr->e_type = 0x0002;
	
	// Update the section offset in the EHDR.
	ehdr->e_shoff = (uint32_t)(ehdr->e_ehsize);
	
	// Write the ELF EHDR to output file.
	fseek(out, 0, SEEK_SET);
	fwrite(ehdr->e_ident, sizeof(ehdr->e_ident), 1, out);
	fwrite16(ehdr->e_type, out);
	fwrite16(ehdr->e_machine, out);
	fwrite32(ehdr->e_version, out);
	fwrite32(ehdr->e_entry, out);
	fwrite32(ehdr->e_phoff, out);
	fwrite32(ehdr->e_shoff, out);
	fwrite32(ehdr->e_flags, out);
	fwrite16(ehdr->e_ehsize, out);
	fwrite16(ehdr->e_phentsize, out);
	fwrite16(ehdr->e_phnum, out);
	fwrite16(ehdr->e_shentsize, out);
	fwrite16(ehdr->e_shnum, out);
	fwrite16(ehdr->e_shstrndx, out);

	// Clean up.
	free(crcs);
	free(fileinfo);
	free(shdr_table);
	free(ehdr);
	free(rpl);
	
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("***********************************************************\n\n");
		printf("rpl2elf v0.0.1 - Convert Wii U RPL/RPX files to ELF.\n");
		printf("               - Written by Hykem (C).\n\n");
		printf("***********************************************************\n\n");
		printf("Usage: rpl2elf [INPUT.RPL/RPX] [OUTPUT.ELF]\n");
		printf("\n");
		return 1;
	}

	// Open files for read/write.
	FILE *in = fopen(argv[1], "rb");
	FILE *out = fopen(argv[2], "wb");
	
	// Convert the file.
	convert_rpl(in, out);
	
	// Clean up.
	fclose(out);
	fclose(in);
	
	return 0;
}