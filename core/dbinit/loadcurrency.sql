-- Home wallet
--
-- Module: Load currency script
--
-- Copyright 2024 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version. See COPYING file for more details.

insert into hw_currency (id, full_name, short_name, abbr, code, seq_order, is_main, is_unit)
    values(1, 'Российский рубль', 'Рубль', '₽', 'RUR', 1, 1, 0);

insert into hw_currency (id, full_name, short_name, abbr, code, seq_order, is_main, is_unit)
    values(2, 'Доллар США', 'Доллары', '$', 'USD', 2, 0, 0);

insert into hw_currency (id, full_name, short_name, abbr, code, seq_order, is_main, is_unit)
    values(3, 'Евро', 'Евро', 'Є', 'EUR', 3, 0, 0);

insert into hw_currency (id, full_name, short_name, abbr, code, seq_order, is_main, is_unit)
    values(4, 'Юань Ренминби', 'Юани', '¥', 'CNY', 4, 0, 0);
    
insert into hw_currency (id, full_name, short_name, abbr, code, seq_order, is_main, is_unit)
    values(5, 'Украинская гривна', 'Гривны', '₴', 'UAH', 5, 0, 0);
    
