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
-- TODO currency rate

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
    -- TODO default unit
    constraint pk_escat primary key(id),
    constraint uk_escat unique(id_ecat, name),
    constraint fk_escat foreign key(id_ecat) references hw_ex_cat(id)
);

-- Accounts (various cards, hard cash, etc.)
-- TODO meta-account (всегда вводить "основной" - на нужном языке)
create table hw_account (
    id integer not null,
    name char(64) not null,
    descr char(256),
    foundation date null,  -- may be null if imported from hb
    id_cur integer null, -- only one currency, in hb - many; may be null if imported from hb
    init_sum integer null, -- in low units (cent, kopeck, pfenning etc.)
    constraint pk_ac primary key(id),
    constraint uk_ac unique(name),
    constraint fk_accur foreign key(id_cur) references hw_currency(id)
    );
-- TODO init sums in separate table
-- TODO account history

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

-- Incomes
create table hw_in_op (
-- TODO source id (see json, for txt - maybe date + line after date, maybe filename and source type)
    id integer not null,
    op_date date not null,
    quantity double,
    -- price integer not null, -- in low units
    amount integer not null, -- in low units
    -- id_subcat integer not null,
    id_ac integer not null,
    id_cur integer not null, -- only one currency, in hb - many
    id_isubcat integer not null,
    id_un integer not null,
    attention integer not null, -- 1 if star setted, 0 if ordinary record
    descr char(256),
    constraint pk_iop primary key(id),
    constraint fk_inac foreign key(id_ac) references hw_account(id),
    constraint fk_incur foreign key(id_cur) references hw_currency(id),
    constraint fk_insub foreign key(id_isubcat) references hw_in_subcat(id),
    constraint fk_inun foreign key(id_un) references hw_units(id)
);

-- Expenses
create table hw_receipt ( -- in shop, rus. чек
-- TODO source id
    id integer not null,
    note char(32),
    total_amount integer not null, -- in low units, on all ex_op in receipt
    id_ac integer not null,
    id_cur integer not null,
    constraint pk_rc primary key(id),
    constraint fk_rcac foreign key(id_ac) references hw_account(id),
    constraint fk_rccur foreign key(id_cur) references hw_currency(id)
);

create table hw_ex_op (
-- TODO source id
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
    constraint pk_eop primary key(id),
    constraint fk_exac foreign key(id_ac) references hw_account(id),
    constraint fk_excur foreign key(id_cur) references hw_currency(id),
    constraint fk_exsub foreign key(id_esubcat) references hw_ex_subcat(id),
    constraint fk_exun foreign key(id_un) references hw_units(id),
    constraint fk_exrc foreign key(id_rc) references hw_receipt(id)
);

-- Transfer
create table hw_transfer_type ( -- currency exchange, deposit into ATM, withdraw from ATM, buy to parents...
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
    constraint pk_tr primary key(id),
    constraint fk_trac_in foreign key(id_ac_in) references hw_account(id),
    constraint fk_trac_out foreign key(id_ac_out) references hw_account(id),
    constraint fk_trac_tt foreign key(id_tt) references hw_transfer_type(id)
);

-- Debtors, Creditors, their names
-- TODO
-- TODO data from bank files (check!)

