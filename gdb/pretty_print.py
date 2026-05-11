import re
import gdb
from itertools import count

def jank_address(oref):
    data_ptr = oref["data"].format_string(raw=True)
    return re.search("(0x[0-9a-fA-F]+)", data_ptr).group(1)

def to_py_string(val):
    address = jank_address(val)
    eval_result = gdb.parse_and_eval(f"((jank::runtime::obj::persistent_string*){address})->data.c_str()")
    match = re.search("0x([0-9a-fA-F]{12}) (.*)", str(eval_result))
    edn_str = match.group(2)
    return edn_str[1:-1].replace('\\"', '"')

def jank_truthy(val):
    address = jank_address(val)
    return int(gdb.parse_and_eval(f"jank_truthy((jank_object_ref){address})")) != 0

def jank_call(var_name, val):
    address = jank_address(val)
    resolve_var = f'jank_eval(jank_read_string_c("{var_name}"))'
    jank_call1 = f'jank_call1({resolve_var}, (jank_object_ref){address})'
    return gdb.parse_and_eval(f'(jank::runtime::object_ref){{(jank::runtime::object*){jank_call1}}}')

def jank_pr_str(val):
    try:
        return to_py_string(jank_call("pr-str", val))
    except Exception as e:
        return str(repr(e))

def jank_type(val):
    try:
        return to_py_string(jank_call("type", val))
    except Exception as e:
        return str(repr(e))

def jank_first(val):
    return jank_call("first", val)

def jank_second(val):
    return jank_call("second", val)

def jank_rest(val):
    return jank_call("rest", val)

def jank_seq(val):
    return jank_call("seq", val)

def jank_map(val):
    return jank_truthy(jank_call("map?", val))

def jank_coll(val):
    return jank_truthy(jank_call("coll?", val))

def jank_seqable(val):
    return jank_truthy(jank_call("seqable?", val))

class jank_print_item:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        try:
            return jank_pr_str(self.val)
        except Exception as e:
            return str(repr(e))

class jank_print_map:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        return jank_pr_str(self.val)
    def children(self):
        try:
            head = jank_seq(self.val)
            for i in count():
                if not jank_truthy(jank_seq(head)):
                    break
                else:
                    entry = jank_first(head)
                    yield jank_pr_str(jank_first(entry)), jank_second(entry)
                    head = jank_rest(head)
        except Exception as e:
            yield str(repr(e))

class jank_print_seq:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        return jank_pr_str(self.val)
    def children(self):
        try:
            head = self.val
            for i in count():
                if not jank_truthy(jank_seq(head)):
                    break
                else:
                    yield str(i), jank_first(head)
                    head = jank_rest(head)
        except Exception as e:
            yield str(repr(e))

def jank_runtime_oref(val):
    try:
        if jank_map(val):
            return jank_print_map(val)
        if jank_seqable(val):
            return jank_print_seq(val)
        else:
            return jank_print_item(val)
    except Exception as e:
        return str(repr(e))

def get_type_name(val):
    return str(val.type.unqualified().strip_typedefs())

def jank_pretty_print(val):
    type_name = get_type_name(val)
    if type_name.startswith("jank::runtime::oref"):
        return jank_runtime_oref(val)

gdb.pretty_printers.append(jank_pretty_print)