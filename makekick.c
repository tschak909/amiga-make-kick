/**
 * @brief   Make a Kickstart Disk from ROM file
 * @author  Thomas Cherryhomes
 * @email   thom dot cherryhomes at gmail dot com
 * @license gpl v. 3, see COPYING, for details.
 */

#include <exec/types.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <graphics/gfxbase.h>
#include <libraries/dos.h>
#include <devices/trackdisk.h>

struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;
struct MsgPort *CreatePort();
struct Window *OpenWindow();

struct IntuiText couldnt_open_write_window=
{
	0,0,
	JAM1,
	8,8,
	NULL,
	"Couldn't Open Write Window!",
	NULL
};

struct IntuiText could_not_alloc_sector=
{
	0,0,
	JAM1,
	8,8,
	NULL,
	"Couldn't allocate sector buffer!",
	NULL
};

struct IntuiText couldnt_open_format_window=
{
	0,0,
	JAM1,
	8,8,
	NULL,
	"Couldn't Open Format Window!",
	NULL
};

struct IntuiText no_mem_for_format=
{
	0,0,
	JAM1,
	8,8,
	NULL,
	"No RAM for format!",
	NULL
};

struct IntuiText are_you_sure_text=
{
	0,0,
	JAM1,
	8,8,
	NULL,
	"This process will erase the target disk!",
	NULL
};

struct IntuiText couldnt_open_trackdisk_text=
{
	0,0,
	JAM1,
	8,8,
	NULL,
	"Couldn't OpenDevice trackdisk.device.",
	NULL
};

struct IntuiText couldnt_create_disk_req_text=
{
	0,0,
	JAM1,
	8,8,
	NULL,
	"Couldn't CreateExtIO() for trackdisk.device",
	NULL
};

struct IntuiText couldnt_open_port_text=
{
	0,0,
	JAM1,
	8,8,
	NULL,
	"Couldn't open Reply port for TrackDisk.device",
	NULL
};

struct IntuiText invalid_unit_no_text=
{
	0,0,
	JAM1,
	8,8,
	NULL,
	"Invalid Unit no. (not 0-3)",
	NULL
};

struct IntuiText couldnt_open_rom_text=
{
	0,0,
	JAM1,
	8,8,
	NULL,
	"Couldn't Open ROM File.",
	NULL
};

struct IntuiText usage_text3=
{
	0,0,
	JAM1,
	15,30,
	NULL,
	"<DFx:> must be a floppy device.",
	NULL
};

struct IntuiText usage_text2=
{
	0,0,
	JAM1,
	15,18,
	NULL,
	"<romfile> must be 256KB in size.",
	&usage_text3
};

struct IntuiText usage_text=
{
	0,0,
	JAM1,
	15,5,
	NULL,
	"1> MakeKick <romfile> <DFx:>",
	&usage_text2
};

struct IntuiText cancel_text=
{
	0,0,
	JAM1,
	6,3,
	NULL,
	"CANCEL",
	NULL
};

struct IntuiText button_text=
{
	0,0,
	JAM1,
	6,3,
	NULL,
	"OK",
	NULL
};

init()
{
	GfxBase = (struct GfxBase *)
		OpenLibrary("graphics.library",0);

	if (!GfxBase)
		return FALSE;

	IntuitionBase = (struct IntuitionBase *)
		OpenLibrary("intuition.library",0);

	if (!IntuitionBase)
		return FALSE;
}

void done()
{
	if (GfxBase)
		CloseLibrary(GfxBase);

	if (IntuitionBase)
		CloseLibrary(IntuitionBase);
}

void show_usage()
{
	AutoRequest(NULL,
		&usage_text,
		NULL,
		&button_text,
		NULL,
		NULL,
		320,88);
}

show_error(t)
struct IntuiText *t;
{
	AutoRequest(NULL,
		t,
		NULL,
		&button_text,
		NULL,
		NULL,
		300,37);
}

are_you_sure()
{
	return AutoRequest(NULL,
			&are_you_sure_text,
			&cancel_text,
			&button_text,
			NULL,
			NULL,
			360,54);	
}

struct NewWindow nwFormat = 
{
	0,0,
	180,32,
	0,1,
	NULL,
	WINDOWDRAG|WINDOWDEPTH|ACTIVATE,
	NULL,
	NULL,
	"Formatting...",
	NULL,
	NULL,
	0,0,320,100,
	WBENCHSCREEN
};

format_disk(disk_req)
struct IOExtTD *disk_req;
{
	int ret = FALSE; /* Error until proven successful. */
	struct Window *w = NULL;
	int track=0;

	char *buf = (char *)
			AllocMem(TD_SECTOR * NUMSECS, MEMF_CHIP|MEMF_CLEAR);

	if (!buf)
	{
		show_error(&no_mem_for_format);
		goto format_bye;
	}

	/* Motor on */
	disk_req->iotd_Req.io_Command = TD_MOTOR;
	disk_req->iotd_Req.io_Length = 1;
	DoIO(disk_req);

	disk_req->iotd_Req.io_Command = TD_FORMAT;
	disk_req->iotd_Req.io_Length = TD_SECTOR * NUMSECS;
	disk_req->iotd_Req.io_Data = (APTR)buf;

	w = OpenWindow(&nwFormat);

	if (!w)
	{
		show_error(&couldnt_open_format_window);
		goto format_bye;	
	}

	SetOPen(w->RPort, 1);
	SetAPen(w->RPort, 2);
	RectFill(w->RPort,8,16,168,24);

	SetOPen(w->RPort, 2);
	SetAPen(w->RPort, 1);

	for (track=0;track<NUMTRACKS;track++)
	{
		RectFill(w->RPort,8,16,track+8,24);
		disk_req->iotd_Req.io_Offset = (TD_SECTOR * NUMSECS) * track;
		DoIO(disk_req);
	}

	ret = TRUE;

format_bye:

	/* Motor off */
	disk_req->iotd_Req.io_Command = TD_MOTOR;
	disk_req->iotd_Req.io_Length = 0;
	DoIO(disk_req);

	if (w)
		CloseWindow(w);

	if (buf)
		FreeMem(buf, TD_SECTOR * NUMSECS);

	return ret;
}

struct NewWindow nwWrite = 
{
	0,0,
	279,32,
	0,1,
	NULL,
	WINDOWDRAG|WINDOWDEPTH|ACTIVATE,
	NULL,
	NULL,
	"Writing KickStart...",
	NULL,
	NULL,
	0,0,320,100,
	WBENCHSCREEN
};

write_kick(fd, disk_req)
int fd;
struct IOExtTD *disk_req;
{
	int ret = FALSE;
	char *buf = NULL;
	int l=512;
	struct Window *w = NULL;

	buf = (char *)AllocMem(TD_SECTOR, MEMF_CHIP|MEMF_CLEAR);

	if (!buf)
	{
		show_error(&could_not_alloc_sector);
		goto write_bye;
	}

	/* Go ahead and fill in the first sector. ;) */
	buf[0]='K'; buf[1]='I'; buf[2]='C'; buf[3]='K';

	w = OpenWindow(&nwWrite);
	if (!w)
	{
		show_error(&couldnt_open_write_window);
		goto write_bye;	
	}

	SetAPen(w->RPort,2);
	SetOPen(w->RPort,1);
	RectFill(w->RPort,8,16,264,24);

	SetAPen(w->RPort,1);
	SetOPen(w->RPort,2);

	/* Motor on */
	disk_req->iotd_Req.io_Command = TD_MOTOR;
	disk_req->iotd_Req.io_Length = 1;
	DoIO(disk_req);

	disk_req->iotd_Req.io_Command = CMD_WRITE;
	disk_req->iotd_Req.io_Length = TD_SECTOR;
	disk_req->iotd_Req.io_Data = (APTR)buf;
	disk_req->iotd_Req.io_Offset = 0;

	while(l)
	{
		int track = disk_req->iotd_Req.io_Offset >> 10;

		RectFill(w->RPort,8,16,track+8,24);

		DoIO(disk_req);
		disk_req->iotd_Req.io_Offset += l;
		l = Read(fd, buf, TD_SECTOR);
	}

	/* Flush out everything. */
	disk_req->iotd_Req.io_Command = CMD_UPDATE;
	DoIO(disk_req);
			
write_bye:

	/* Motor off */
	disk_req->iotd_Req.io_Command = TD_MOTOR;
	disk_req->iotd_Req.io_Length = 0;
	DoIO(disk_req);

	if (w)
		CloseWindow(w);

	if (buf)
		FreeMem(buf, TD_SECTOR);

	return ret;
}

makekick(fn, df)
char *fn, *df;
{
	struct MsgPort *port = NULL;
	struct IOExtTD *disk_req = NULL;
	int unit = df[2]-'0';
	int fd = Open(fn, MODE_OLDFILE);
	int deviceOpened = 0;

	if (!fd)
	{
		show_error(&couldnt_open_rom_text);
		goto bail;
	}

	if (unit<0 || unit>3)
	{
		show_error(&invalid_unit_no_text);
		goto bail;
	}

	port = CreatePort(0, 0);

	if (!port)
	{
		show_error(&couldnt_open_port_text);
		goto bail;
	}

	disk_req = (struct IOExtTD *)
		CreateExtIO(port, sizeof(struct IOExtTD));

	if (!disk_req)
	{
		show_error(&couldnt_create_disk_req_text);
		goto bail;	
	}

	if (OpenDevice(TD_NAME,unit,disk_req,0))
	{
		show_error(&couldnt_open_trackdisk_text);
		goto bail;
	}
	else
		deviceOpened = 1;

	if (are_you_sure())
		goto bail;

	if (format_disk(disk_req))
		write_kick(fd,disk_req);
	
bail:
	if (deviceOpened)
		CloseDevice(disk_req);

	deviceOpened = 0;

	if (disk_req)
		DeleteExtIO(disk_req, sizeof(struct IOExtTD));

	if (port)
		DeletePort(port);

	if (fd)
		Close(fd);
}

main(argc, argv)
int argc;
char *argv[];
{
	if (init())
	{
		if (argc < 3)
		{
			show_usage();
		}
		else
			makekick(argv[1], argv[2]);
	}

	done();

	return 0;
}
