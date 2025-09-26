-- Home wallet
--
-- Module: Cach database generation script
--
-- Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version. See COPYING file for more details.

-- must be compatible with PostgreSQL 8+ and SQLite 3

-- TODO schema name

-- Currency unit
create table hw_currency (
    id integer not null,
    full_name char(64) not null, -- US dollar
    short_name char(32) not null, -- dollar
    abbr char(4) not null, -- $
    code char(4) not null, -- USD
    seq_order integer, -- human-readable order
    is_main integer not null, -- 1 if main, 0 otherwise
    is_unit integer not null, -- 1 if other currency calc through this, 0 if this calc through other currency
    descr char(256),
    constraint pk_cur primary key(id),
    constraint uk_cur1 unique(full_name),
    constraint uk_cur2 unique(abbr),
    constraint uk_cur3 unique(code)
);

create table hw_curr_rate (
    id integer not null,
    ch_date date not null,
    id_cur_low integer null,
    id_cur_high integer null,
    rate double,
    constraint pk_cr primary key(id),
    constraint fk_crcurl foreign key(id_cur_low) references hw_currency(id),
    constraint fk_crcurh foreign key(id_cur_high) references hw_currency(id)
);

-- Units (kg, liter, day, etc.)
create table hw_unit (
    id integer not null,
    name char(64) not null,
    short_name char(16) not null,
    descr char(256),
    constraint pk_un primary key(id),
    constraint uk_un1 unique(name),
    constraint uk_un2 unique(short_name)
    );

-- Incomes categories & subcategories
create table hw_in_cat (
    id integer not null,
    name char(64) not null,
    descr char(256),
    constraint pk_icat primary key(id),
    constraint uk_icat unique(name)
);

create table hw_in_subcat (
    id integer not null,
    id_icat integer not null,
    name char(64) not null,
    descr char(256),
    constraint pk_iscat primary key(id),
    constraint uk_iscat unique(id_icat, name),
    constraint fk_iscat foreign key(id_icat) references hw_in_cat(id)
);

-- Expenses categories & subcategories
create table hw_ex_cat (
    id integer not null,
    name char(64) not null,
    descr char(256),
    constraint pk_ecat primary key(id),
    constraint uk_ecat unique(name)
);

create table hw_ex_subcat (
    id integer not null,
    id_ecat integer not null,
    name char(64) not null,
    descr char(256),
    id_un_default integer null,
    constraint pk_escat primary key(id),
    constraint uk_escat unique(id_ecat, name),
    constraint fk_escat foreign key(id_ecat) references hw_ex_cat(id),
    constraint fk_esund foreign key(id_un_default) references hw_unit(id)
);

-- Accounts (various cards, hard cash, etc.)
-- TODO meta-account (всегда вводить "основной" - на нужном языке)
create table hw_account (
    id integer not null,
    name char(64) not null,
    descr char(256),
    foundation date null,  -- may be null if imported from hb
    constraint pk_ac primary key(id),
    constraint uk_ac unique(name)
);

create table hw_acc_init ( -- Start balance for various currencies
    id integer not null,
    id_ac integer not null,
    id_cur integer null,
    init_sum integer null, -- in low units (cent, kopeck, pfennig etc.)
    constraint pk_ai primary key(id),
    constraint uk_ai unique(id_ac, id_cur),
    constraint fk_aiac foreign key(id_ac) references hw_account(id),
    constraint fk_aicur foreign key(id_cur) references hw_currency(id)
);

create table hw_acc_hist ( -- account history
    id integer not null,
    ch_date date not null,
    id_ai integer not null,
    sum integer null, -- in low units
    constraint pk_ah primary key(id),
    constraint uk_ah unique(ch_date, id_ai),
    constraint fk_ahai foreign key(id_ai) references hw_acc_init(id)
);

-- Import & audit
create table hw_imp_file (
    id integer not null,
    imp_date date not null,
    filename char(128) not null,
    filetype char(8) not null, -- XMHWA, XMHBK, JSFNS, XLS, TXTCF, QIF...
    descr char(256),
    constraint pk_imp primary key(id),
    constraint uk_imp unique(filename)
);

create table hw_alias (
    id integer not null,
    -- ONE of next ids will be not null
    id_ac integer null,
    id_cur integer null,
    id_un integer null,
    id_icat integer null,
    id_ecat integer null,
    id_isubcat integer null,
    id_esubcat integer null,
    id_tt integer null,
    -- END of ids
    pattern char(64) not null,
    to_descr char(64) not null,
    constraint pk_ali primary key(id),
    constraint uk_ali unique(pattern)
);

-- TODO audit

-- Incomes
create table hw_in_op (
    id integer not null,
    op_date date not null,
    quantity double,
    -- price integer not null, -- in low units
    amount integer not null, -- in low units
    -- id_subcat integer not null,
    id_ac integer not null,
    id_cur integer not null, -- only one currency, in hb - many
    id_isubcat integer not null,
    id_un integer null,
    attention integer not null, -- 1 if star setted, 0 if ordinary record
    descr char(256),
    id_imp integer null,
    uid_imp char(64), -- _id for JSON, line after date for TXT...
    constraint pk_iop primary key(id),
    constraint fk_inac foreign key(id_ac) references hw_account(id),
    constraint fk_incur foreign key(id_cur) references hw_currency(id),
    constraint fk_insub foreign key(id_isubcat) references hw_in_subcat(id),
    constraint fk_inun foreign key(id_un) references hw_units(id),
    constraint fk_inimp foreign key(id_imp) references hw_imp_file(id)
);

-- Expenses
create table hw_receipt ( -- in shop, rus. чек
    id integer not null,
    note char(32),
    total_amount integer not null, -- in low units, on all ex_op in receipt
    id_ac integer not null,
    id_cur integer not null,
    id_imp integer null,
    uid_imp char(64), -- _id for JSON, line after date for TXT...
    constraint pk_rc primary key(id),
    constraint fk_rcac foreign key(id_ac) references hw_account(id),
    constraint fk_rccur foreign key(id_cur) references hw_currency(id),
    constraint fk_rcimp foreign key(id_imp) references hw_imp_file(id)
);

create table hw_ex_op (
    id integer not null,
    op_date date not null,
    quantity double,
    -- price integer not null, -- in low units
    amount integer not null, -- in low units, after discount, if discount present
    -- id_subcat integer not null,
    id_ac integer not null,
    id_cur integer not null, -- only one currency, in hb - many
    id_esubcat integer not null,
    id_un integer null,
    id_rc integer null, -- expence may be separate, but may be contained in receipt
    discount integer, -- absolute value in low units (not percent)
    attention integer not null, -- 1 if star setted, 0 if ordinary record
    descr char(256),
    id_imp integer null,
    uid_imp char(64), -- _id for JSON, line after date for TXT...
    constraint pk_eop primary key(id),
    constraint fk_exac foreign key(id_ac) references hw_account(id),
    constraint fk_excur foreign key(id_cur) references hw_currency(id),
    constraint fk_exsub foreign key(id_esubcat) references hw_ex_subcat(id),
    constraint fk_exun foreign key(id_un) references hw_units(id),
    constraint fk_exrc foreign key(id_rc) references hw_receipt(id),
    constraint fk_eximp foreign key(id_imp) references hw_imp_file(id)
);

-- Transfer & currency exchange
create table hw_transfer_type ( -- deposit into ATM, withdraw from ATM, buy to parents...
    id integer not null,
    name char(64) not null,
    descr char(256),
    constraint pk_tt primary key(id),
    constraint uk_tt unique(name)
);

create table hw_transfer (
    id integer not null,
    op_date date not null,
    amount integer not null, -- in low units
    id_cur integer not null,
    id_ac_in integer not null,
    id_ac_out integer not null,
    id_tt integer not null,
    descr char(256),
    id_imp integer null,
    uid_imp char(64), -- _id for JSON, line after date for TXT...
    constraint pk_tr primary key(id),
    constraint fk_trcur foreign key(id_cur) references hw_currency(id),
    constraint fk_trac_in foreign key(id_ac_in) references hw_account(id),
    constraint fk_trac_out foreign key(id_ac_out) references hw_account(id),
    constraint fk_trac_tt foreign key(id_tt) references hw_transfer_type(id),
    constraint fk_trimp foreign key(id_imp) references hw_imp_file(id)
);

create table hw_curr_exch (
    id integer not null,
    op_date date not null,
    id_ac integer not null,
    id_cur_in integer not null,
    id_cur_out integer not null,
    amount_in integer not null, -- in low units
    amount_out integer not null, -- in low units
    descr char(256),
    id_imp integer null,
    uid_imp char(64), -- _id for JSON, line after date for TXT...
    constraint pk_ce primary key(id),
    constraint fk_ceac foreign key(id_ac) references hw_account(id),
    constraint fk_cecur_in foreign key(id_cur_in) references hw_currency(id),
    constraint fk_cecur_out foreign key(id_cur_out) references hw_currency(id),
    constraint fk_ceimp foreign key(id_imp) references hw_imp_file(id)
);

-- Debtors, Creditors, their names
-- TODO
-- TODO data from bank files (check!)

