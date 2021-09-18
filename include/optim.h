//  Copyright (C) 2021 Benjamin Stürz
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef FILE_OPTIM_H
#define FILE_OPTIM_H
#include "expr.h"
#include "stmt.h"
#include "ir.h"

struct expression* optim_expr(struct expression*, struct scope*);
struct statement* optim_stmt(struct statement*);
struct ir_node* optim_ir_nodes(struct ir_node*);

// target-specific IR optimizations
bool target_optim_ir(struct ir_node**);

// ... that are performed, after all other IR optimizations are done.
bool target_post_optim_ir(struct ir_node**);

#endif /* FILE_OPTIM_H */
