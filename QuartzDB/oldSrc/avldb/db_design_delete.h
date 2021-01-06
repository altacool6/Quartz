#ifndef __DB_DESIGN_DELETE_H__
#define __DB_DESIGN_DELETE_H__
int DeleteObjectTableRecord(unsigned long vRowid);
int DeleteReferenceTableRecord(unsigned long vRowid);
int DeletePhotoTableRecord(unsigned long vRowid);
int DeleteAlbumTableRecord(unsigned long vRowid);
int DeleteMusicTableRecord(unsigned long vRowid);
#endif
