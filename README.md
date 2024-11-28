# amiga-make-kick
Commodore Amiga: Make Kickstart floppy from KickStart ROM file

## What is it?

Just as it says on the tin, this program can take a 256KB Amiga Kickstart
ROM, and write it as a KickStart floppy, which can boot in Amiga 1000
systems.

It serves as an example of how to implement software that utilizes the
trackdisk.device.

## Usage

```sh
1> MakeKick <kick.rom> <DFx:>
```

To write the KickStart ROM kickV27.3.rom to drive DF1: you would do:

```sh
1> MakeKick kickV27.3.rom DF1:
```

If incorrect parameters are given, a requester showing usage is displayed.

If any of the parameters are incorrect, a requester showing remediation is displayed.

### WARNING

The destination disk will be formatted, and the KickStart ROM will comprise the first 23 tracks.

## Author

Thomas Cherryhomes <thom dot cherryhomes at gmail dot com>

## License

GPL v. 3, see LICENSE for details.
