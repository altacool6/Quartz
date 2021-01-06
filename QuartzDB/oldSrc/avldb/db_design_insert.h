#ifndef __DB_DESIGN_INSERT_H__
#define __DB_DESIGN_INSERT_H__

#ifdef MTP_OBJECT_TABLE_EXTENSION
unsigned long InsertObjectTableRecord(	unsigned long 		vParentUid, 
										unsigned long 		vUid, 
										unsigned char 	vProperties, 
										unsigned short 	vFileType, 
										unsigned short* 	pObjectName,
										unsigned short*	pAliasName,
										unsigned long		vFileSize,
										unsigned long		vDateCreated,
										unsigned long		vRowid,
										unsigned long 		vPuoid1,
										unsigned long 		vPuoid2);
#else
unsigned long InsertObjectTableRecord(	unsigned long 		vParentUid, 
										unsigned long 		vUid, 
										unsigned char 	vProperties, 
										unsigned short 	vFileType, 
										unsigned short* 	pObjectName);
#endif
unsigned long InsertReferenceTableRecord(	unsigned long 		vParentUid, 
											unsigned long 		vChildUid, 
											unsigned short 	vFileFormat);
unsigned long InsertPhotoTableRecord(	unsigned long		vWidth, 
										unsigned long		vHeight, 
										unsigned short	vSampleFormat, 
										unsigned long 		vSampleWidth, 
										unsigned long 		vSampleHeight, 
										unsigned long 		vPosition, 
										unsigned long 		vSampleSize, 
										unsigned long 		vUid);
unsigned long InsertAlbumTableRecord(	unsigned short* pArtist, 
										unsigned short* pAlbum, 
										unsigned short* pGenre, 
										unsigned short   vSampleFormat, 
										unsigned long 	   vSampleWidth, 
										unsigned long    vSampleHeight, 
										unsigned long    vPosition, 
										unsigned long    vSampleSize, 
										unsigned long    vStartClusA, 
										unsigned long    vUid);
unsigned long InsertMusicTableRecord(	unsigned short* pArtist, 
										unsigned short* pAlbum, 
										unsigned short* pGenre, 
										unsigned short* pTitle, 
										unsigned long 	   vDuration, 
										unsigned short   vRating, 
										unsigned long 	   vUseCount, 
										unsigned short   vFileFormat,
										unsigned short   vTrackNumber,
										unsigned char    vDrm,
										unsigned char    vLyric,
										unsigned char    vPurchase,
										unsigned short   vProtectionStatus,
										unsigned long    vSampleRate,
										unsigned long    vBitRate,
										unsigned char    vChangedFlag,
										unsigned long    vAudioWAVECodec,
										unsigned long    vStartClusM,
										unsigned long    vStartClusA,
										unsigned long    vAlbumArtPos,
										unsigned short* pOrgReleaseDate,
										unsigned short* pAlbumArtist,
										unsigned long    vUid,
										unsigned long    vRaingTime);
#endif
