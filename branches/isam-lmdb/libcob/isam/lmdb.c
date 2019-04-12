/*
   Copyright (C) 2002-2012, 2014-2019 Free Software Foundation, Inc.
   Written by Keisuke Nishida, Roger While, Simon Sobisch, Ron Norman

   This file is part of GnuCOBOL.

   The GnuCOBOL runtime library is free software: you can redistribute it
   and/or modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   GnuCOBOL is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GnuCOBOL.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "../../libcob.h"
#include "../coblocal.h"
#include "isam.h"

#if WITH_LMDB

/* Local variables */


static int indexed_open		(cob_file *, char *, const int, const int);
static int indexed_close	(cob_file *, const int);
static int indexed_start	(cob_file *, const int, cob_field *);
static int indexed_read		(cob_file *, cob_field *, const int);
static int indexed_read_next	(cob_file *, const int);
static int indexed_write	(cob_file *, const int);
static int indexed_delete	(cob_file *);
static int indexed_rewrite	(cob_file *, const int);
static int indexed_file_delete  (cob_file *, const char *filename);

const struct cob_fileio_funcs indexed_funcs = {
	indexed_open,
	indexed_close,
	indexed_start,
	indexed_read,
	indexed_read_next,
	indexed_write,
	indexed_rewrite,
	indexed_delete,
	indexed_file_delete
};

static char		*db_buff = NULL;
static const char	**db_data_dir = NULL;

#define INTTYPES_H_MISSING
#include <lmdb.h>
#include <stdbool.h>
#ifndef _WIN32	/* correct would be a check for HAVE_SYS_FILE_H */
#include <libgen.h>
#include <sys/file.h>
#endif

#include <sys/stat.h>
#define MDB_MAX_MAP_INC 1073741824

#define	cob_dbtsize_t		size_t

struct indexed_file {
	MDB_env		*db_env;
	MDB_dbi		**db;		/* Database handlers */
	MDB_txn		*txn;
	MDB_cursor	**cursor;
	MDB_val		key;
	MDB_val		data;
        int             fd;
	char		*filename;	/* Needed for record locks */
	unsigned char	*last_key;	/* The last key written */
	unsigned char	*temp_key;	/* Used for temporary storage */
	unsigned char	**last_readkey;	/* The last key read */
	cob_u32_t	*last_dupno;	/* The last number of duplicates read */
	cob_u32_t	*rewrite_sec_key;
	int		maxkeylen;
	int		primekeylen;
	unsigned char* savekey;	/* Work area for saving key value */
	unsigned char* suppkey;	/* Work area for saving key value */
	unsigned char* saverec;	/* For saving copy of record */
	size_t		key_index;
	cob_u32_t	write_cursor_open;
	cob_u32_t	record_locked;
	cob_u32_t	filenamelen;
	cob_u32_t	db_flags;
	cob_u32_t	txn_flags;
	cob_u32_t	env_flags;
	struct flock    lock;
};

#define DBT_SET(key_or_data,fld)			\
	key_or_data.mv_data = fld->data;			\
	key_or_data.mv_size = (cob_dbtsize_t) fld->size

#define DBT_SET_SIZED(key_or_data,data,size)			\
	key_or_data.mv_data = data;			\
	key_or_data.mv_size = (cob_dbtsize_t) size

#define xxxMDB_SET(p,f) \
	DBT_SET(p->key,f->keys[0].field); \
	DBT_SET(p->data,f->record)

/* Local functions */

static int
dummy_delete (cob_file *f)
{
	COB_UNUSED (f);

	return COB_STATUS_91_NOT_AVAILABLE;
}

static int
dummy_rnxt_rewrite (cob_file *f, const int opt)
{
	COB_UNUSED (f);
	COB_UNUSED (opt);

	return COB_STATUS_91_NOT_AVAILABLE;
}

static int
dummy_read (cob_file *f, cob_field *key, const int read_opts)
{
	COB_UNUSED (f);
	COB_UNUSED (key);
	COB_UNUSED (read_opts);

	return COB_STATUS_91_NOT_AVAILABLE;
}

static int
dummy_start (cob_file *f, const int cond, cob_field *key)
{
	COB_UNUSED (f);
	COB_UNUSED (cond);
	COB_UNUSED (key);

	return COB_STATUS_91_NOT_AVAILABLE;
}

/* Is given key data all SUPPRESS char,
   returns 1 if key has all SUPPRESS char */
static int
db_suppresskey (cob_file *f, int idx)
{
	unsigned char ch_sprs;
	int 	i,len;
	struct indexed_file	*p;

	if (!f->keys[idx].tf_suppress) {
		return 0;
	}
	ch_sprs = f->keys[idx].char_suppress & 0xFF;
	p = f->file;
	len = db_savekey(f, p->suppkey, f->record->data, idx);
	for (i = 0; i < len; i++) {
		if (p->suppkey[i] != ch_sprs)
			return 0;
	}
	return 1;
}

static void
db_setkey (cob_file *f, int idx)
{
	struct indexed_file *p = f->file;
	int len;

	memset (p->savekey, 0, p->maxkeylen);
	len = db_savekey (f, p->savekey, f->record->data, idx);

	p->key.mv_data = p->savekey;
	p->key.mv_size = len;
} 

/*
static int
db_cmpkey (cob_file *f, unsigned char *keyarea, unsigned char *record, int idx, int partlen)
{
	int sts, part, totlen;
	size_t cl;

	if (partlen <= 0) {
		partlen = db_keylen(f, idx);
	}
	if (f->keys[idx].count_components > 0) {
		totlen = 0;
		for (part = 0; part < f->keys[idx].count_components && partlen > 0; part++) {
			cl = partlen > f->keys[idx].component[part]->size ? f->keys[idx].component[part]->size : partlen;
			sts = memcmp (keyarea + totlen, 
				record + (f->keys[idx].component[part]->data - f->record->data), cl);
			if (sts != 0) {
				return sts;
			}
			totlen += f->keys[idx].component[part]->size;
			partlen -= f->keys[idx].component[part]->size;
		}
		return 0;
	}
	cl = partlen > f->keys[idx].field->size ? f->keys[idx].field->size : partlen;
	return memcmp (keyarea, record + (f->keys[idx].field->data - f->record->data), cl);
}
*/

void
isam_sync (cob_file *f) {}


/* INDEXED */

# if 0	/* TODO: Come back and implement locking */
static int
lock_record (cob_file *f, const char *key, const unsigned int keylen)
{

}

static int
test_record_lock (cob_file *f, const char *key, const unsigned int keylen)
{

}

static int
unlock_record (cob_file *f)
{
}

#endif	/* TODO */


/* Get the next number in a set of duplicates */
static unsigned int
get_dupno (cob_file *f, const cob_u32_t i)
{
	struct indexed_file	*p = f->file;
	int			ret;
	cob_u32_t		dupno = 0;
	
	// Using a nested transaction so we don't mess up the write transacion in indexed_write
	MDB_txn    *txn;
	MDB_cursor *cursor;
	txn    = cob_malloc(sizeof(MDB_txn *));
	cursor = cob_malloc(sizeof(MDB_cursor *));
	
	db_setkey(f, i);
	//memcpy (p->temp_key, p->key.mv_data, p->key.mv_size); //REVISIT:
	memcpy (p->temp_key, p->key.mv_data, (size_t)p->maxkeylen);
	mdb_txn_begin(p->db_env, p->txn , 0, &txn);
	mdb_cursor_open(txn, *p->db[i], &cursor);
	ret = mdb_cursor_get(cursor,&p->key,&p->data,MDB_SET_RANGE);
	//while (ret == 0 && memcmp(p->key.mv_data, p->temp_key, p->key.mv_size) == 0) {
	while (ret == 0 && memcmp(p->key.mv_data, p->temp_key, (size_t)p->key.mv_size) == 0) { //REVISIT: Cast may be unnecessary
		//memcpy(&dupno,(cob_u8_ptr)p->data.mv_data + f->keys[0].field->size,sizeof(unsigned int));
		memcpy(&dupno,(cob_u8_ptr)p->data.mv_data + p->primekeylen, sizeof(unsigned int));
		ret = mdb_cursor_get(cursor,&p->key,&p->data,MDB_NEXT);
	}
	mdb_cursor_close(cursor);
	mdb_txn_commit(txn);
	return ++dupno;
}

/* read file with all alternate keys that don't allow duplicates
   to check if records exist already, returns 1 if true */
static int
check_alt_keys (cob_file *f, const int rewrite)
{
	struct indexed_file  *p = f->file;
	size_t i;
	
	// Transaction is inherited from caller
	for (i = 1; i < f->nkeys; ++i) {
		if (!f->keys[i].tf_duplicates) {
			int ret;
			db_setkey(f, i);
			ret = mdb_get(p->txn,*p->db[i],&p->key,&p->data);
			if (ret == 0) {
				if (rewrite) {
					if (db_cmpkey(f, p->data.mv_data, f->record->data, 0, 0)) {
						return 1;
					}
				} else {
					return 1;
				}
			}
		}
	}
	return 0;
}

static int
mdb_cob_status( int mdb_error ) 
{
	switch (mdb_error) {
	case MDB_KEYEXIST:
		return COB_STATUS_22_KEY_EXISTS;			
	case MDB_NOTFOUND:
		return COB_STATUS_23_KEY_NOT_EXISTS;
	case EACCES:
		return COB_STATUS_37_PERMISSION_DENIED;

	case MDB_BAD_DBI:
	case MDB_BAD_RSLOT:
	case MDB_BAD_TXN:
	case MDB_BAD_VALSIZE:
	case MDB_CORRUPTED:
	case MDB_CURSOR_FULL:
	case MDB_DBS_FULL:
	case MDB_INCOMPATIBLE:
	case MDB_INVALID:
	case MDB_MAP_FULL:
	case MDB_MAP_RESIZED:
	case MDB_PAGE_FULL:
	case MDB_PAGE_NOTFOUND:
	case MDB_PANIC:
	case MDB_READERS_FULL:
	case MDB_TLS_FULL:
	case MDB_TXN_FULL:
	case MDB_VERSION_MISMATCH:
		/* fall through */
	default:
		WARN("%s", mdb_strerror(mdb_error));
	}
	return COB_STATUS_30_PERMANENT_ERROR;
}

static int
indexed_write_internal (cob_file *f, const int rewrite, const int opt, unsigned int ds)
{
	struct indexed_file	*p = f->file;
	cob_u32_t	i, len;
	cob_u32_t	dupno;
	cob_u32_t flags;
	int	ret = COB_STATUS_00_SUCCESS;

	COB_UNUSED(opt);

	if ((ret = mdb_txn_begin(p->db_env, NULL, p->txn_flags, &p->txn)) != MDB_SUCCESS) {
		return ret;
	}
	
	if ((ret = mdb_cursor_open(p->txn, *p->db[0], &p->cursor[0])) != MDB_SUCCESS) {
		mdb_txn_abort(p->txn);
		return ret;
	}

	/* Cursors. */
	for (i = 1; i < f->nkeys; i++) {
		if ((ret = mdb_cursor_open(p->txn, *p->db[i], &p->cursor[i])) != MDB_SUCCESS) {
			mdb_txn_abort(p->txn);
			return ret;
		}
	}

	/* Check duplicate alternate keys */
	if (f->nkeys > 1 && !rewrite) {
		if (check_alt_keys (f, 0)) {
			mdb_txn_abort(p->txn);
			return MDB_KEYEXIST;
		}
		db_setkey(f, 0);
	}

	/* Position write cursor */
	if (rewrite) {
		if ((ret = mdb_cursor_get(p->cursor[0],&p->key, &p->data, MDB_SET)) == MDB_SUCCESS) {
			mdb_txn_abort(p->txn);
			return MDB_KEYEXIST;
		}
	}

	p->data.mv_data = f->record->data;
	p->data.mv_size = (size_t) f->record->size;
	
	flags = 0; //(rewrite) ? MDB_CURRENT : MDB_NOOVERWRITE;
	if ((ret = mdb_cursor_put(p->cursor[0], &p->key, &p->data, flags)) != MDB_SUCCESS) {
		mdb_txn_abort(p->txn);
		return ret;
	} 

	if (f->nkeys == 1) {
		return mdb_txn_commit(p->txn);
	}

	/* Write secondary keys */
	p->data = p->key;
	for (i = 1; i < f->nkeys; ++i) {
		if (rewrite && ! p->rewrite_sec_key[i]) {
			continue;
		}
		if (db_suppresskey(f, i)) {
			continue;
		}
		// Set the key of the secondary key
		db_setkey(f, i);
		if (f->keys[i].tf_duplicates)	{
			flags = 0;
			dupno = get_dupno(f, i);
			if (dupno > 1) {
				ret = COB_STATUS_02_SUCCESS_DUPLICATE;
			}
			len = db_savekey(f, p->temp_key, f->record->data, 0);
			p->data.mv_data = p->temp_key;
			p->data.mv_size = len;
			memcpy (((char *)(p->data.mv_data)) + p->data.mv_size, &dupno, sizeof (unsigned int));
			p->data.mv_size += sizeof(unsigned int);
		} else {
			len = db_savekey(f, p->temp_key,  f->record->data, 0);
			p->data.mv_data = p->temp_key;
			p->data.mv_size = len
			flags = MDB_NOOVERWRITE;
			dupno = 0;
		}
		db_setkey (f, i);

		if ((ret = mdb_cursor_put(p->cursor[i],&p->key,&p->data,flags)) != MDB_SUCCESS) {
			//mdb_txn_abort(p->txn);
			mdb_txn_commit(p->txn);
			return ret;
		}
	}

	if ((ret = mdb_txn_commit(p->txn)) == MDB_SUCCESS) {
		if (f->keys[i].tf_duplicates) {
			ds = COB_STATUS_02_SUCCESS_DUPLICATE;
		}
	}

	return ret; 
}

static int
indexed_start_internal (cob_file *f, const int cond, cob_field *key,
			const int read_opts, const int test_lock)
{
	struct indexed_file	*p = f->file;
	int			len, fullkeylen, partlen;
	int			ret = 0;
	int     rc = 0;
	cob_u32_t		dupno = 0;
	int			key_index;

	COB_UNUSED (read_opts);
	COB_UNUSED (test_lock);

	/* Look up for the key */
	key_index = db_findkey(f, key, &fullkeylen, &partlen);
	if (key_index < 0) {
		return COB_STATUS_23_KEY_NOT_EXISTS;
	}
	p->key_index = (unsigned int)key_index;

	/* Set the key to search */
	db_setkey (f, p->key_index);
	p->key.mv_size = partlen;
	
	/* Start the transaction */
	if ((ret = mdb_txn_begin(p->db_env, NULL, p->txn_flags, &p->txn)) != MDB_SUCCESS) {
		return mdb_cob_status(ret);
	}
	
	/* Open a cursor for an alternate key */
	if (p->key_index !=0) {
		if ((ret = mdb_cursor_open(p->txn, *p->db[0], &p->cursor[0])) != MDB_SUCCESS) {
			mdb_txn_abort(p->txn);
			return mdb_cob_status(ret);
		}
	}
	
	/* Create a cursor */
	if ((ret = mdb_cursor_open(p->txn,*p->db[p->key_index], &p->cursor[p->key_index])) != MDB_SUCCESS) {
		return mdb_cob_status(ret);
	}

	if (cond == COB_FI) {
		ret = mdb_cursor_get(p->cursor[p->key_index], &p->key, &p->data, MDB_FIRST);
	} else if (cond == COB_LA) {
		ret = mdb_cursor_get(p->cursor[p->key_index], &p->key, &p->data, MDB_LAST);
	}  else {
		ret = mdb_cursor_get(p->cursor[p->key_index], &p->key, &p->data, MDB_SET_RANGE);
	}

	switch (cond) {
	case COB_EQ:
		if (ret == MDB_SUCCESS) {
			//ret = memcmp (p->key.mv_data, key->data, key->size);
			ret = db_cmpkey (f, p->key.mv_data, f->record->data, p->key_index, partlen);
		}
		break;
	case COB_LT:
		if (ret != MDB_SUCCESS) {
			ret = mdb_cursor_get(p->cursor[p->key_index], &p->key, &p->data, MDB_LAST);
		} else {
			ret = mdb_cursor_get(p->cursor[p->key_index], &p->key, &p->data, MDB_PREV);
		}
		break;
	case COB_LE:
		if (ret != MDB_SUCCESS) {
			ret = mdb_cursor_get(p->cursor[p->key_index], &p->key, &p->data, MDB_LAST);
		// } else if (memcmp(p->key.mv_data, key->data, key->size) !=0) {
		} else if (db_cmpkey(f, p->key.mv_data, f->record->data, p->key_index, partlen) !=0) {
			ret = mdb_cursor_get(p->cursor[p->key_index], &p->key, &p->data, MDB_PREV);
		} else if (f->keys[p->key_index].flag) {
			ret = mdb_cursor_get(p->cursor[p->key_index], &p->key, &p->data, MDB_NEXT_NODUP);
			if (ret != MDB_SUCCESS) {
				ret = mdb_cursor_get(p->cursor[p->key_index], &p->key, &p->data, MDB_LAST);
			} else {
				ret = mdb_cursor_get(p->cursor[p->key_index], &p->key, &p->data, MDB_PREV);
			}
		}
		break;
	case COB_GT:
		// while (ret == 0 && memcmp(p->key.mv_data, key->data, key->size) == 0) {
		while (ret == MDB_SUCCESS && db_cmpkey (f, p->key.mv_data, f->record->data, p->key_index, partlen)  == MDB_SUCCESS) {
			ret = mdb_cursor_get(p->cursor[p->key_index], &p->key, &p->data, MDB_NEXT);
		}
		break;
	case COB_GE:
		/* nothing */
		break;
	case COB_FI:
		/* nothing */
		break;
	case COB_LA:
		/* nothing */
		break;
	}

	if (ret == MDB_SUCCESS && p->key_index > 0) {
		/* Temporarily save alternate key */
		len = p->key.mv_size;
		memcpy(p->temp_key, p->key.mv_data, len);
		if (f->keys[p->key_index].tf_duplicates) {
			memcpy(&dupno, (cob_u8_ptr)p->data.mv_data + p->primekeylen, sizeof(unsigned int));
		}
		p->key.mv_data = p->data.mv_data;
		p->key.mv_size = p->primekeylen;
		ret = mdb_get(p->txn,*p->db[0],&p->key,&p->data);
	}

#if 0	/* TODO: Come back to lock test */
	if (ret == 0 && test_lock) {

	}
#endif

	if (ret == MDB_SUCCESS) {
		if (p->key_index == 0) {
			memcpy (p->last_readkey[0], p->key.mv_data, p->primekeylen);
		} else {
			memcpy (p->last_readkey[p->key_index],
				p->temp_key, db_keylen(f, p->key_index));
			memcpy (p->last_readkey[p->key_index + f->nkeys], p->key.mv_data,	p->primekeylen);
			if (f->keys[p->key_index].tf_duplicates) {
				p->last_dupno[p->key_index] = dupno;
			}
		}
	}

	mdb_cursor_close(p->cursor[p->key_index]);
	if ((rc = mdb_txn_commit(p->txn)) != MDB_SUCCESS) {
		return mdb_cob_status(rc);
	}
	return (ret == MDB_SUCCESS) ? COB_STATUS_00_SUCCESS : COB_STATUS_23_KEY_NOT_EXISTS;

}

static int
indexed_delete_internal (cob_file *f, const int rewrite)
{
	struct indexed_file	*p = f->file;
	size_t			i, len;
	MDB_val			prim_key;
	int			ret;
	cob_u32_t		flags;
	COB_UNUSED(flags);

	flags = 0;
	if ((ret = mdb_txn_begin(p->db_env, NULL, p->txn_flags, &p->txn)) != MDB_SUCCESS) {
		return mdb_cob_status(ret);
	}

	if ((ret = mdb_cursor_open(p->txn, *p->db[0], &p->cursor[0])) != MDB_SUCCESS) {
		return mdb_cob_status(ret);
	}
	
#if 0	/* TODO: Come back and implement locking */
	if (p->db_env != NULL) {
		unlock_record (f);
	}
#endif

	/* Find the primary key */
	if (f->access_mode != COB_ACCESS_SEQUENTIAL) {
		db_setkey(f, 0);
	}

	if ((ret = mdb_cursor_get(p->cursor[0],&p->key,&p->data,MDB_SET)) != MDB_SUCCESS) {
		if (f->access_mode != COB_ACCESS_SEQUENTIAL) {
			mdb_txn_abort(p->txn);
		}
		return mdb_cob_status(ret);
	}
	
#if 0	/* TODO: Come back and implement locking */
	if (p->db_env != NULL) {
		ret = test_record_lock (f, p->key.data, p->key.size);
		if (ret) {
			if (close_cursor) {
				p->cursor[0]->c_close (p->cursor[0]);
				p->cursor[0] = NULL;
				p->write_cursor_open = 0;
			}
			return COB_STATUS_51_RECORD_LOCKED;
		}
	}
#endif
	prim_key = p->key;
	memcpy(p->saverec, p->data.mv_data, p->data.mv_size);    /* Save old record image  */
	memcpy(p->temp_key, prim_key.mv_data, prim_key.mv_size); /* Save primary key value */
	prim_key.mv_data = p->temp_key;

	/* Delete the secondary keys */
	for (i = 1; i < f->nkeys; ++i) {
		len = db_savekey(f, p->suppkey, p->data.mv_data, i);
		memset(p->savekey, 0, p->maxkeylen);
		len = db_savekey(f, p->savekey, p->saverec, i);
		p->key.mv_data = p->savekey;
		p->key.mv_size = (size_t) len;
		p->key.mv_data = (char *)p->key.mv_data;
		/* rewrite: no delete if secondary key is unchanged */
		if (rewrite) {
			//p->rewrite_sec_key[i] = memcmp (p->key.mv_data, f->keys[i].field->data, (size_t)p->key.mv_size);
			p->rewrite_sec_key[i] = db_cmpkey (f, p->suppkey, f->record->data, i, 0);
			if (!p->rewrite_sec_key[i]) {
				continue;
			}
		}
		if (!f->keys[i].tf_duplicates) {
			if ((ret = mdb_del(p->txn,*p->db[i],&p->key,&p->data)) != MDB_SUCCESS) {
				mdb_txn_abort(p->txn);
				return mdb_cob_status(ret);
			}
		} else {
			MDB_val sec_key = p->key;

			if (( ret = mdb_cursor_open(p->txn,*p->db[i], &p->cursor[i])) != MDB_SUCCESS) {
				mdb_txn_abort(p->txn);
				return mdb_cob_status(ret);
			}

			if (mdb_cursor_get(p->cursor[i],&p->key,&p->data,MDB_SET_RANGE) == MDB_SUCCESS) {
				while (sec_key.mv_size == p->key.mv_size
				    && memcmp (p->key.mv_data, sec_key.mv_data, (size_t)sec_key.mv_size) == 0) {
					if (memcmp (p->data.mv_data, prim_key.mv_data, (size_t)prim_key.mv_size) == 0) {
						mdb_cursor_del (p->cursor[i],0);
					}
					if (mdb_cursor_get(p->cursor[i],&p->key,&p->data,MDB_NEXT) != MDB_SUCCESS) {
						break;
					}
				}
			}
			mdb_cursor_close(p->cursor[i]);
		}
	}

	/* Delete the record */
	if ((ret = mdb_cursor_del(p->cursor[0], 0)) != MDB_SUCCESS) {
		return mdb_cob_status(ret);
	}

	if ((ret = mdb_txn_commit(p->txn)) != MDB_SUCCESS) {
		return mdb_cob_status(ret);	
	}
	return COB_STATUS_00_SUCCESS;
}

static int
mdb_resize_env (MDB_env* e)
{
	MDB_envinfo ei;
	mdb_env_info (e, &ei);
	return mdb_env_set_mapsize (e, ei.me_mapsize * 2 );
}

/* Delete file */
static int
indexed_file_delete (cob_file *f, const char *filename)
{
	COB_UNUSED (f);
	COB_UNUSED (filename);
	return COB_STATUS_00_SUCCESS;
}

/* OPEN INDEXED file */

static void
indexed_file_free(struct indexed_file* p)
{ }

static int
indexed_open (cob_file *f, char *filename, const int mode, const int sharing)
{
	/* Note filename points to file_open_name */
	/* cob_chk_file_mapping manipulates file_open_name directly */

	struct indexed_file	*p;
	size_t i, j;
	size_t maxsize;
	int    ret = 0;
	int nonexistent = 0;
	int lock_mode;

	cob_chk_file_mapping ();
	if (cobsetptr->db_home == NULL) {
	  static const char name[] = ".";
	  cobsetptr->db_home = cob_malloc(sizeof(name));
	  strcpy(cobsetptr->db_home,name);
	}

	if (getenv("MDB_NO_SHARED_FS_CHK") == 0) {
		struct stat sb;
		char dir[ 1 + strlen(filename) ];
		
		sprintf(dir, "%s", filename);
		if ((stat(dirname(dir), &sb) == -1) || (!S_ISDIR(sb.st_mode))) {
			return COB_STATUS_30_PERMANENT_ERROR;
		}

		char *devname;
		bool is_local = local_file(sb.st_dev, &devname);
		if (!is_local) {
			fprintf(stderr,"Shared filesystem detected!\n");
			return COB_STATUS_30_PERMANENT_ERROR;
		}
	}

	if (db_nofile (filename)) {
		nonexistent = 1;
		if (mode != COB_OPEN_OUTPUT && f->flag_optional == 0) {
			return COB_STATUS_35_NOT_EXISTS;
		}
	}

	p = cob_malloc (sizeof (struct indexed_file));

	switch (mode) {
	case COB_OPEN_INPUT:
		p->env_flags |= MDB_RDONLY;
		p->txn_flags |= MDB_RDONLY;
		break;
	case COB_OPEN_OUTPUT:
		p->db_flags |= MDB_CREATE;
		break;
	case COB_OPEN_I_O:
	case COB_OPEN_EXTEND:
		p->db_flags |= MDB_CREATE;
		break;
	}

	p->db              = cob_malloc(sizeof(MDB_dbi *) * f->nkeys);
	p->cursor          = cob_malloc(sizeof(MDB_cursor *) * f->nkeys);
	p->last_readkey    = cob_malloc(sizeof(unsigned char *) * 2 * f->nkeys);
	p->last_dupno      = cob_malloc(sizeof(unsigned int * ) * 2 * f->nkeys);
	p->rewrite_sec_key = cob_malloc(sizeof(int) * f->nkeys);
	maxsize = p->primekeylen = db_keylen(f, 0);
	for (i = 1; i < f->nkeys; ++i) {
		j = db_keylen(f, i);
		if (j > maxsize)
			maxsize = j;
	}
	p->maxkeylen = maxsize;

	if ((ret = mdb_env_create(&p->db_env)) != MDB_SUCCESS) {
		indexed_file_free(p);
		return mdb_cob_status(ret);
		//return COB_STATUS_30_PERMANENT_ERROR;
	}

	if (f->nkeys > 1) {
		if ((ret = mdb_env_set_maxdbs(p->db_env,f->nkeys)) != MDB_SUCCESS ) {
			mdb_env_close(p->db_env);
			p->db_env = NULL;
			indexed_file_free(p);
			return mdb_cob_status(ret);
			//return COB_STATUS_30_PERMANENT_ERROR;
		}
	}

	if (nonexistent) {
		if (f->flag_optional) {
			if (mode == COB_OPEN_INPUT) {
				f->open_mode = mode;
				f->flag_nonexistent = 1;
				f->flag_end_of_file = 1;
				f->flag_begin_of_file = 1;
				return COB_STATUS_05_SUCCESS_OPTIONAL;
			}
		}

		if ((ret = mkdir(filename, S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH | S_IXOTH)) != 0) {
			switch (ret) {
			case EACCES:
				indexed_file_free(p);
				return COB_STATUS_37_PERMISSION_DENIED;
			default:
				indexed_file_free(p);
				return COB_STATUS_30_PERMANENT_ERROR;
			}
		}
	}

	if ((ret = mdb_env_open(p->db_env, filename, p->env_flags, 0770)) != MDB_SUCCESS) {
		mdb_env_close(p->db_env);
		p->db_env = NULL;
		indexed_file_free(p);
		return mdb_cob_status(ret);
		//return COB_STATUS_30_PERMANENT_ERROR;
	}

	if (sharing) {
		if (mode == COB_OPEN_OUTPUT || mode == COB_OPEN_EXTEND ||
		    (f->lock_mode & COB_FILE_EXCLUSIVE) ||
		    (mode == COB_OPEN_I_O && !f->lock_mode)) {
			lock_mode = F_WRLCK;
		} else {
			lock_mode = F_RDLCK;
		}
               
		if ((ret = mdb_env_get_fd(p->db_env, &p->fd)) != MDB_SUCCESS) {
			mdb_env_close(p->db_env);
			p->db_env = NULL;
			indexed_file_free(p);
			return mdb_cob_status(ret);
		}

		memset((void *)&p->lock, 0, sizeof (struct flock));
		p->lock.l_type = lock_mode;
		p->lock.l_whence = SEEK_SET;
		p->lock.l_start = 0;
		p->lock.l_len = 0;
		p->lock.l_pid = getpid();
		errno = 0;
		int fcd = 0;
		fcd = fcntl(p->fd, F_SETLK, &p->lock);
		if (fcd < 0) {
			ret = errno;
			p->fd = -1;
			switch (ret) {
			case EACCES:
			case EAGAIN:
			case EDEADLK:
				return COB_STATUS_61_FILE_SHARING;
			default:
				return COB_STATUS_30_PERMANENT_ERROR;
			}
		}

	}

	for (i = 0; i < f->nkeys; i++) {
		if (f->keys[i].field->size > maxsize) {
			maxsize = f->keys[i].field->size;
		}
	}

	if ((ret = mdb_txn_begin(p->db_env, NULL, p->txn_flags, &p->txn)) != MDB_SUCCESS) {
		mdb_env_close(p->db_env);
		p->db_env = NULL;
		indexed_file_free(p);
		return mdb_cob_status(ret);
		//return COB_STATUS_30_PERMANENT_ERROR;
	}

	for (i = 0; i < f->nkeys; i++) {
		if (i == 0) {
			snprintf(runtime_buffer, (size_t)COB_FILE_MAX, "%s", filename);
		} else {
			snprintf(runtime_buffer, (size_t)COB_FILE_MAX, "%s.%d", filename, (int)i);
		}
		
		p->db[i] = cob_malloc(sizeof(MDB_dbi *));
		if ((ret = mdb_open(p->txn, 
			(f->nkeys == 1) ? NULL : runtime_buffer,
			(p->db_flags|((f->keys[i].tf_duplicates)?(MDB_DUPSORT):0)) , p->db[i])) != MDB_SUCCESS) {
			int j;
			for (j = 0; j < i; ++j) {
				mdb_dbi_close(p->db_env,*p->db[j]);
			}
			mdb_env_close(p->db_env);
			p->db_env = NULL;
			indexed_file_free(p);
			return mdb_cob_status(ret);
			//return COB_STATUS_30_PERMANENT_ERROR;
		}
		p->last_readkey[i] = cob_malloc (maxsize);
		p->last_readkey[f->nkeys + i] = cob_malloc (maxsize);
	}

	p->temp_key = cob_malloc (maxsize + sizeof (unsigned long));
	p->savekey  = cob_malloc (maxsize + sizeof (unsigned long));
	p->suppkey  = cob_malloc (maxsize + sizeof (unsigned long));
	p->saverec  = cob_malloc (f->record_max + sizeof (unsigned long));

	f->file = p;
	p->key_index = 0;
	p->last_key = NULL;

	memset ((void *)&p->key, 0, sizeof (MDB_val));
	memset ((void *)&p->data, 0, sizeof (MDB_val));
	p->filenamelen = strlen(filename);
	p->filename = cob_malloc(p->filenamelen + 1);
	memcpy (p->filename, filename, strlen(filename));
	p->write_cursor_open = 0;
	p->record_locked = 0;
	f->open_mode = mode;

	db_setkey (f, 0);

	if ((ret = mdb_cursor_open(p->txn, *p->db[0], &p->cursor[0])) != 0) {
		return mdb_cob_status(ret);
	}

	if ((ret = mdb_cursor_get(p->cursor[0],&p->key,&p->data,MDB_FIRST)) == MDB_SUCCESS) {
		memcpy (p->last_readkey[0], p->key.mv_data, p->key.mv_size);
		if (p->data.mv_data != NULL
			&& p->data.mv_size > 0
			&& p->data.mv_size > f->record_max) {
				return COB_STATUS_39_CONFLICT_ATTRIBUTE;
		}
	} else {
		p->data.mv_data = NULL;
	}

	mdb_cursor_close(p->cursor[0]);
	if ((ret = mdb_txn_commit(p->txn)) != MDB_SUCCESS) {
			int j;
			for (j = 0; j < i; ++j) {
				mdb_dbi_close(p->db_env,*p->db[j]);
			}
			mdb_env_close(p->db_env);
			p->db_env = NULL;
			indexed_file_free(p);
			return mdb_cob_status(ret);
	}

	f->open_mode = mode;
	if (f->flag_optional && nonexistent) {
		return COB_STATUS_05_SUCCESS_OPTIONAL;
	}
	return COB_STATUS_00_SUCCESS;
}

/* Close the INDEXED file */

static int
indexed_close (cob_file *f, const int opt)
{
	struct indexed_file *p = f->file;
	int i;
	COB_UNUSED(opt);

	for (i = 0; i < f->nkeys; i++) {
		mdb_close(p->db_env, *p->db[i]);
	}
	mdb_env_close(p->db_env);
	p->db_env = NULL;
	if (p) cob_free(p);
	return COB_STATUS_00_SUCCESS;
}

/* START INDEXED file with positioning */

static int
indexed_start (cob_file *f, const int cond, cob_field *key)
{
	return indexed_start_internal (f, cond, key, 0, 0);
}

/* Random READ of the INDEXED file  */

static int
indexed_read (cob_file *f, cob_field *key, const int read_opts)
{
	/* TODO: Return and implement locking. */

	struct indexed_file	*p;
	int			ret;
	int			db_opts;
	int			test_lock = 0;

	p = f->file;
	db_opts = read_opts;

	if ((ret = indexed_start_internal (f, COB_EQ, key, db_opts, test_lock)) != COB_STATUS_00_SUCCESS) {
		return ret;
	}

	f->record->size = p->data.mv_size;
	if (f->record->size > f->record_max) {
		f->record->size = f->record_max;
		ret = COB_STATUS_43_READ_NOT_DONE;
	} else {
		ret = COB_STATUS_00_SUCCESS;
	}
	memcpy (f->record->data, p->data.mv_data, f->record->size);

	return ret;
}

/* Sequential READ of the INDEXED file */

static int
indexed_read_next (cob_file *f, const int read_opts)
{
	struct		indexed_file  *p = f->file;
	int		ret;
	int		read_nextprev;
	cob_u32_t	nextprev;
	int		file_changed;
	cob_u32_t	dupno = 0;
	nextprev = MDB_NEXT;
	file_changed = 0;
	
#if 0	/* TODO: Come back and implement locking. */
	if (db_env != NULL) {
		if (f->open_mode != COB_OPEN_I_O ||
				(f->lock_mode & COB_FILE_EXCLUSIVE)) {
			db_opts &= ~COB_READ_LOCK;
		} else if ((f->lock_mode & COB_LOCK_AUTOMATIC) &&
					!(db_opts & COB_READ_NO_LOCK)) {
			db_opts |= COB_READ_LOCK;
		}
		unlock_record (f);
	} else {
		db_opts &= ~COB_READ_LOCK;
	}
#endif

	/*
	if (unlikely (read_opts & COB_READ_PREVIOUS)) {
		nextprev = f->flag_end_of_file? MDB_LAST : MDB_PREV;
	} else {
		nextprev = f->flag_begin_of_file? MDB_FIRST : MDB_NEXT;
	}
	*/
	if (unlikely (read_opts & COB_READ_PREVIOUS)) {
		if (f->flag_end_of_file) {
			nextprev = MDB_LAST;
		} else {
			nextprev = MDB_PREV;
		}
	} else if (f->flag_begin_of_file) {
		nextprev = MDB_FIRST;
	}

	if ((ret = 	mdb_txn_begin(p->db_env, NULL, MDB_RDONLY, &p->txn)) != MDB_SUCCESS) {
		return mdb_cob_status(ret);
	}

	/* The open cursor makes this function atomic */
	if (p->key_index != 0) {
		if ((ret = mdb_cursor_open(p->txn, *p->db[0], &p->cursor[0])) != MDB_SUCCESS) {
			return mdb_cob_status(ret);
		}
	}

	if ((ret = mdb_cursor_open(p->txn, *p->db[p->key_index], &p->cursor[p->key_index])) != MDB_SUCCESS) {
		return mdb_cob_status(ret);
	}
	
	if (f->flag_first_read) {
		/* Data is read in indexed_open or indexed_start */
		if (p->data.mv_data == NULL || (f->flag_first_read == 2 &&
				nextprev == MDB_PREV)) {
			mdb_cursor_close(p->cursor[p->key_index]);
			if (p->key_index != 0) {
				mdb_cursor_close(p->cursor[0]);
			}
			mdb_txn_abort(p->txn);
			return COB_STATUS_10_END_OF_FILE;
		}

		/* Check if previously read data still exists */
		p->key.mv_size = (size_t) db_keylen(f,p->key_index);
		p->key.mv_data = p->last_readkey[p->key_index];

		ret = mdb_cursor_get(p->cursor[p->key_index],&p->key,&p->data,MDB_SET);
		if (!ret && p->key_index > 0) {
			if (f->keys[p->key_index].tf_duplicates) {
				memcpy (&dupno, (cob_u8_ptr)p->data.mv_data + p->primekeylen, sizeof(unsigned int));
				while (ret == 0 &&
							memcmp (p->key.mv_data, p->last_readkey[p->key_index], (size_t)p->key.mv_size) == 0 &&
							dupno < p->last_dupno[p->key_index]) {
					ret = mdb_cursor_get(p->cursor[p->key_index],&p->key,&p->data,MDB_NEXT);
					memcpy (&dupno, (cob_u8_ptr)p->data.mv_data + p->primekeylen, sizeof(unsigned int));
				}
				if (ret == 0 &&
						memcmp (p->key.mv_data, p->last_readkey[p->key_index], (size_t)p->key.mv_size) == 0 &&
						dupno == p->last_dupno[p->key_index]) {
					ret = memcmp (p->last_readkey[p->key_index + f->nkeys], p->data.mv_data, p->primekeylen);
				} else {
					ret = 1;
				}
			} else {
				ret = memcmp (p->last_readkey[p->key_index + f->nkeys], p->data.mv_data, p->primekeylen);
			}
			if (!ret) {
				p->key.mv_size = (size_t) p->primekeylen;
				p->key.mv_data = p->last_readkey[p->key_index + f->nkeys];
				ret = mdb_get(p->txn,*p->db[0],&p->key,&p->data);
			}
		}
		file_changed = ret;

#if 0 /* TODO: Come back and implement locking. */
		if (db_env != NULL && !file_changed) {
			if (!(db_opts & COB_READ_IGNORE_LOCK)) {
				ret = test_record_lock (f, p->key.mv_data, p->key.mv_size);
				if (ret) {
					mdb_cursor_close(p->cursor[p->key_index])
					if (p->key_index != 0) {
						mdb_cursor_close(p->cursor[0]);
					}
					return COB_STATUS_51_RECORD_LOCKED;
				}
			}
			if (db_opts & COB_READ_LOCK) {
				ret = lock_record (f, p->key.mv_data, p->key.mv_size);
				if (ret) {
					mdb_cursor_close(p->cursor[p->key_index])
					if (p->key_index != 0) {
						mdb_cursor_close(p->cursor[0]);
					}
					return COB_STATUS_51_RECORD_LOCKED;
				}
			}
		}
#endif
	}

	if (!f->flag_first_read || file_changed) {
		if (nextprev == MDB_FIRST || nextprev == MDB_LAST) {
			read_nextprev = 1;
		} else {
			p->key.mv_size = (size_t) db_keylen(f,p->key_index);
			p->key.mv_data = p->last_readkey[p->key_index];
			ret = mdb_cursor_get(p->cursor[p->key_index],&p->key,&p->data,MDB_SET_RANGE);
			/* ret != 0 possible, records may be deleted since last read */
			if (ret != 0) {
				if (nextprev == MDB_PREV) {
					nextprev = MDB_LAST;
					read_nextprev = 1;
				} else {
					mdb_cursor_close(p->cursor[p->key_index]);
					if (p->key_index != 0) {
						mdb_cursor_close(p->cursor[0]);
					}
					mdb_txn_commit(p->txn);
					return COB_STATUS_10_END_OF_FILE;
				}
			} else {
				if (memcmp (p->key.mv_data, p->last_readkey[p->key_index], (size_t)p->key.mv_size) == 0) {
					if (p->key_index > 0 && f->keys[p->key_index].tf_duplicates) {
						memcpy (&dupno, (cob_u8_ptr)p->data.mv_data + p->primekeylen, sizeof(unsigned int));
						while (ret == 0 &&
						memcmp (p->key.mv_data, p->last_readkey[p->key_index], (size_t)p->key.mv_size) == 0 &&
						dupno < p->last_dupno[p->key_index]) {
							ret = mdb_cursor_get(p->cursor[p->key_index],&p->key,&p->data,MDB_NEXT);
							memcpy (&dupno, (cob_u8_ptr)p->data.mv_data + f->keys[0].field->size, sizeof(unsigned int));
						}
						if (ret != 0) {
							if (nextprev == MDB_PREV) {
								nextprev = MDB_LAST;
								read_nextprev = 1;
							} else {
								mdb_cursor_close(p->cursor[p->key_index]);
								if (p->key_index != 0) {
									mdb_cursor_close(p->cursor[0]);
								}
								mdb_txn_commit(p->txn);
								return COB_STATUS_10_END_OF_FILE;
							}
						} else {
							if (memcmp (p->key.mv_data, p->last_readkey[p->key_index], (size_t)p->key.mv_size) == 0 &&
								dupno == p->last_dupno[p->key_index]) {
								read_nextprev = 1;
							} else {
								if (nextprev == MDB_PREV) {
									read_nextprev = 1;
								} else {
									read_nextprev = 0;
								}
							}
						}
					} else {
						read_nextprev = 1;
					}
				} else {
					if (nextprev == MDB_PREV) {
						read_nextprev = 1;
					} else {
						read_nextprev = 0;
					}
				}
			}
		}
		
		if (read_nextprev) {
			ret = mdb_cursor_get(p->cursor[p->key_index],&p->key,&p->data,nextprev);
			if (ret != 0) {
				mdb_cursor_close(p->cursor[p->key_index]);
				if (p->key_index != 0) {
					mdb_cursor_close(p->cursor[0]);
				}
				mdb_txn_commit(p->txn);
				return COB_STATUS_10_END_OF_FILE;
			}
		}
		
		if (p->key_index > 0) {
			/* Temporarily save alternate key */
			memcpy (p->temp_key, p->key.mv_data, (size_t)p->key.mv_size);
			if (f->keys[p->key_index].tf_duplicates) {
				memcpy (&dupno, (cob_u8_ptr)p->data.mv_data + p->primekeylen, sizeof(unsigned int));
			}
			p->key.mv_data = p->data.mv_data;
			p->key.mv_size = p->primekeylen;
			if (mdb_get(p->txn,*p->db[0],&p->key,&p->data) != 0) {
				mdb_cursor_close(p->cursor[p->key_index]);
				if (p->key_index != 0) {
					mdb_cursor_close(p->cursor[0]);
				}
				mdb_txn_commit(p->txn);
				return COB_STATUS_23_KEY_NOT_EXISTS;
			}
		}
		
#if 0	/* TODO: Come back and implement locking */
		if (db_env != NULL) {
			if (!(db_opts & COB_READ_IGNORE_LOCK)) {
				ret = test_record_lock (f, p->key.mv_data, p->key.mv_size);
				if (ret) {
					p->cursor[p->key_index]->c_close (p->cursor[p->key_index]);
					p->cursor[p->key_index] = NULL;
					if (p->key_index != 0) {
						p->cursor[0]->c_close (p->cursor[0]);
						p->cursor[0] = NULL;
					}
					return COB_STATUS_51_RECORD_LOCKED;
				}
			}
			if (db_opts & COB_READ_LOCK) {
				ret = lock_record (f, p->key.mv_data, p->key.mv_size);
				if (ret) {
					mdb_cursor_close(p->cursor[p->key_index])
					if (p->key_index != 0) {
						mdb_cursor_close(p->cursor[0]);
					}
					return COB_STATUS_51_RECORD_LOCKED;
				}
			}
		}
#endif

		if (p->key_index == 0) {
			memcpy (p->last_readkey[0], p->key.mv_data, (size_t)p->key.mv_size);
		} else {
			memcpy (p->last_readkey[p->key_index], p->temp_key,
						db_keylen(f, p->key_index));
			memcpy (p->last_readkey[p->key_index + f->nkeys], p->key.mv_data, p->primekeylen);
			if (f->keys[p->key_index].tf_duplicates) {
				p->last_dupno[p->key_index] = dupno;
			}
		}
	}

	mdb_cursor_close(p->cursor[p->key_index]);
	if (p->key_index != 0) {
		mdb_cursor_close(p->cursor[0]);
	}
	mdb_txn_commit(p->txn);

	f->record->size = p->data.mv_size;
	if (f->record->size > f->record_max) {
		f->record->size = f->record_max;
		ret = COB_STATUS_43_READ_NOT_DONE;
	} else {
		ret = COB_STATUS_00_SUCCESS;
	}
	memcpy (f->record->data, p->data.mv_data, f->record->size);

	return ret;
}

/* WRITE to the INDEXED file  */

static int
indexed_write (cob_file *f, const int opt)
{
	struct indexed_file	*p;
	int rc = 0;
	unsigned int cs = COB_STATUS_00_SUCCESS;

	if (f->flag_nonexistent) {
		return COB_STATUS_48_OUTPUT_DENIED;
	}
	p = f->file;

	/* Check record key */
	db_setkey (f, 0);
	if (!p->last_key) {
		p->last_key = cob_malloc ((size_t)p->maxkeylen);
	} else if (f->access_mode == COB_ACCESS_SEQUENTIAL &&
			 memcmp (p->last_key, p->key.mv_data, (size_t)p->key.mv_size) > 0) {
		return COB_STATUS_21_KEY_INVALID;
	}
	memcpy (p->last_key, p->key.mv_data, (size_t)p->key.mv_size);
	while ((rc = indexed_write_internal(f, 0, opt, cs)) != MDB_SUCCESS) {
		if (rc == MDB_MAP_FULL) {
			mdb_resize_env(p->db_env);
		} else {
			return mdb_cob_status(rc);
		}
	}
	return (cs != COB_STATUS_00_SUCCESS) ? cs : COB_STATUS_00_SUCCESS;
}

/* DELETE record from the INDEXED file  */

static int
indexed_delete (cob_file *f)
{
	if (f->flag_nonexistent) {
		return COB_STATUS_49_I_O_DENIED;
	}
	return indexed_delete_internal (f, 0);
}

/* REWRITE record to the INDEXED file  */

static int
indexed_rewrite (cob_file *f, const int opt)
{
	if (f->flag_nonexistent) {
		return COB_STATUS_49_I_O_DENIED;
	}
	struct indexed_file	*p = f->file;
	int			ret;

	/* Check duplicate alternate keys */
	if (check_alt_keys (f, 1)) {
		return COB_STATUS_22_KEY_EXISTS;
	}

	/* Delete the current record */
	if ((ret = indexed_delete_internal (f, 1)) != COB_STATUS_00_SUCCESS) {
		return ret;
	}

	/* Write data */
	db_setkey (f, 0);
	while ((ret = indexed_write_internal (f, 1, opt, 0)) != MDB_SUCCESS) {
		if (ret == MDB_MAP_FULL) {
			mdb_resize_env(p->db_env);
		} else {
			return mdb_cob_status(ret);
		}
	}
	return COB_STATUS_00_SUCCESS;
}

/* Initialization/Termination
   cobsetpr-values with type ENV_PATH or ENV_STR
   like db_home and cob_file_path are taken care in cob_exit_common()!
*/

void
cob_exit_fileio_aide (void)
{
  return;
}

void
cob_init_fileio_epilogue (cob_global *lptr, cob_settings *sptr)
{
	db_data_dir = NULL;
	db_buff = cob_malloc (COB_SMALL_BUFF);
}

void cob_file_unlock_internal (cob_file *f) {}




#endif

