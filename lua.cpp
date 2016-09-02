#include <iostream>
#include <string.h>
using namespace std;

extern "C"
{
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

void print_stack(lua_State* l) {

  int i;
  int top = lua_gettop(l);
  for (i = 1; i <= top; i++) {

    int type = lua_type(l, i);
    switch (type) {

      case LUA_TSTRING:
        std::cout << "\"" << lua_tostring(l, i) << "\"";
        break;

      case LUA_TBOOLEAN:
        std::cout << (lua_toboolean(l, i) ? "true" : "false");
        break;

      case LUA_TNUMBER:
        std::cout << lua_tonumber(l, i);
        break;

      case LUA_TLIGHTUSERDATA:
        std::cout << "lightuserdata:" << lua_touserdata(l, i);
        break;

      case LUA_TUSERDATA:
      {
        std::cout << "userdata";
        break;
      }

      default:
        std::cout << lua_typename(l, type);
        break;

    }
    std::cout << " ";
  }
  std::cout << std::endl;
}

void print_table(lua_State* L, int index, std::string str){
    std::cout << "__Print_Table__" << str << std::endl;
    index = lua_gettop(L) + index + 1;
    lua_pushnil(L);
    while (lua_next(L, index) != 0) {
        /* 此时栈上 -1 处为 value, -2 处为 key */
        std::cout << lua_tostring(L, -2) << " - " << std::endl;
        lua_pop(L, 1);
    }
}

std::string main_module_name = "sol.main";
std::string test_module_name = "sol.test";

static int l_sin (lua_State *L) {
    double d = luaL_checknumber(L, 1);//这个函数是为了检测传入的参数是否是正确的类型值，否则Lua直接报错
    lua_pushnumber(L, d*2);
    return 1; /* number of results */
}

static int l_dir (lua_State *L) {
    std::cout << "Hello World" <<endl;
    return 0;
}

static int l_test (lua_State *L) {
    std::cout << "Test" <<endl;
    return 0;
}

static const struct luaL_reg funcs [] = {
    {"dir", l_dir},
    {"sin", l_sin},
    {NULL, NULL}
};

static const struct luaL_reg testfuncs [] = {
    {"show", l_test},
    {NULL, NULL}
};

//The first method to implement the class binding
class Sprite
{
public:
    Sprite(int x, int y):
        x(x),
        y(y){
            this->name = "Sprite";
        }
    int x, y;
    std::string name;
};

static int newSprite(lua_State* l) {
    int n = lua_gettop(l);
    if(n != 2)
        return luaL_error(l, "Got %d arguments expected 2", n);
    Sprite **s = (Sprite **)lua_newuserdata(l, sizeof(Sprite *));
    int x = luaL_checknumber(l, 1);
    int y = luaL_checknumber(l, 2);
    *s = new Sprite(x, y);
    std::cout << *s << std::endl;
    lua_getglobal(l ,"Sprite");
    lua_setmetatable(l, -2);
    return 1;
}

static int spriteX(lua_State* l) {
    int n = lua_gettop(l);
    if(n != 1)
        return luaL_error(l, "Got %d arguments expected 1", n);
    auto sp = (Sprite**)lua_touserdata(l, 1);
    std::cout << *sp << std::endl;
    luaL_argcheck(l, sp != NULL, 1, "`Sprite' expected");
    std::cout << "C++ Sprite.x : " << (*sp)->x << std::endl;
    lua_pushnumber(l, (*sp)->x);
    return 1;
}

static const luaL_Reg gSpriteFuncs[] = {
    {"new", newSprite},
    {"getx", spriteX},
    {NULL, NULL}
};

void registerSprite(lua_State *l){
    luaL_register(l, "Sprite", gSpriteFuncs);  
    lua_pushvalue(l, -1);
    lua_setfield(l, -2, "__index");  
    lua_pop(l, 1);
}
//End

//Second method
static int newSprite2(lua_State *L) 
{
    int n = lua_gettop(L);  // Number of arguments
    if (n != 3) 
        return luaL_error(L, "Got %d arguments expected 3 (class, x, y)", n); 
    // First argument is now a table that represent the class to instantiate        
    luaL_checktype(L, 1, LUA_TTABLE);   
    
    lua_newtable(L);      // Create table to represent instance //sprite x y table

    // Set first argument of new to metatable of instance
    lua_pushvalue(L,1);       //sprite x y table sprite

    lua_setmetatable(L, -2);    //sprite x y table

    // Do function lookups in metatable
    lua_pushvalue(L,1); //sprite x y table sprite
    
    /*
    luaL_getmetatable(L, "Lusion.Sprite"); //sprite x y table sprite Lusion.Sprite
    lua_getfield(L, -1, "destory"); //sprite x y table sprite Lusion.Sprite destroy
    lua_setfield(L, -3, "__gc");    //sprite x y table sprite Lusion.Sprite
    lua_pop(L, 1);  //sprite x y table sprite
    */

    lua_setfield(L, 1, "__index");  //sprite x y table


    

    // Allocate memory for a pointer to to object
    Sprite **s = (Sprite **)lua_newuserdata(L, sizeof(Sprite *));

    int x = luaL_checknumber (L, 2);      
    int y = luaL_checknumber (L, 3);

    *s = new Sprite(x, y);

    std::cout << "new sprite2 : " << *s <<endl;
    
    // Get metatable 'Lusion.Sprite' store in the registry
    luaL_getmetatable(L, "Lusion.Sprite");  //sprite x y table userdata Lusion.Sprite

    // Set user data for Sprite to use this metatable
    lua_setmetatable(L, -2);       //sprite x y table userdata
    
    // Set field '__self' of instance table to the sprite user data
    lua_setfield(L, -2, "__self");  //sprite x y table
    
  return 1; 
}

Sprite* checkSprite(lua_State* L, int index)
{
  void* ud = 0;
  luaL_checktype(L, index, LUA_TTABLE); 
  lua_getfield(L, index, "__self");
  ud = luaL_checkudata(L, -1, "Lusion.Sprite");
  luaL_argcheck(L, ud != 0, 1, "`Lusion.Sprite' expected");  
  
  return *((Sprite**)ud);      
}

static int spriteX2(lua_State* l) {
    int n = lua_gettop(l);
    if(n != 1)
        return luaL_error(l, "Got %d arguments expected 1", n);
    //auto sp = (Sprite**)lua_touserdata(l, 1);
    Sprite* sp = checkSprite(l, 1);
    std::cout << sp << std::endl;
    luaL_argcheck(l, sp != NULL, 1, "`Sprite' expected");
    std::cout << "C++ Sprite.x : " << sp->x << std::endl;
    lua_pushnumber(l, sp->x);
    return 1;
}

static int SpriteDestory(lua_State* l) {
    int n = lua_gettop(l);
    if(n != 1)
        return luaL_error(l, "Got %d arguments expected 1", n);
    Sprite** sp = (Sprite**)luaL_checkudata(l, 1, "Lusion.Sprite");
    std::cout << "C++ Sprite : " << *sp << " deleted " << std::endl;
    delete *sp;
    return 0;
}

static const luaL_Reg gSpriteFuncs2[] = {
    {"new", newSprite2},
    {"getx", spriteX2},
    {NULL, NULL}
};

static const luaL_Reg gDestroySpriteFuncs[] = {
    {"__gc", SpriteDestory},
    {NULL, NULL}
};

void registerSprite2(lua_State *L)
{  
  // Register metatable for user data in registry
  luaL_newmetatable(L, "Lusion.Sprite");
  luaL_register(L, 0, gDestroySpriteFuncs);      
  //luaL_register(L, 0, gSpriteFuncs2);
  print_table(L, -1, "register2");   
  lua_pushvalue(L,-1);
  lua_setfield(L,-2, "__index");  
  
  // Register the base class for instances of Sprite
  luaL_register(L, "Sprite2", gSpriteFuncs2);
}
//End

int main(){
    lua_State *l = luaL_newstate();  
    luaL_openlibs(l);

    lua_newtable(l); //all_userdata
    lua_newtable(l); //all_userdata metatable
    lua_pushstring(l, "v"); //all_userdata metatable “v”
    lua_setfield(l, -2, "__mode"); //all_userdata metatable
    lua_setmetatable(l, -2); //all_userdata
    lua_setfield(l, LUA_REGISTRYINDEX, "sol.all_userdata"); //  存入Registry
    lua_newtable(l);    //userdata_tables
    lua_setfield(l, LUA_REGISTRYINDEX, "sol.userdata_tables");//  存入Registry
    lua_newtable(l);    //sol
    lua_setglobal(l, "sol");    //
    luaL_register(l, main_module_name.c_str(), funcs);  //funcTable
    lua_pop(l, 1);  //
    luaL_register(l, test_module_name.c_str(), testfuncs);  //funcTable
    lua_pop(l, 1);  //
    lua_getglobal(l, "sol");    //sol
    print_table(l, -1, "Sol");
    lua_getfield(l, -1, "main");    //sol nil
    print_stack(l);
    print_table(l, -1, "main"); //table里有sin 和 dir
    lua_setfield(l, LUA_REGISTRYINDEX, main_module_name.c_str());   //也保存到Registry里
    lua_pop(l, 1);

    //registerSprite(l);
    registerSprite2(l);

    if(luaL_loadfile(l, "class1.lua") || lua_pcall(l, 0, 0, 0)){
        std::cout << "connot run lua file" << lua_tostring(l, -1) << std::endl;
        lua_pop(l, 1);
    }

    int index = -1;
    //push main
    lua_getfield(l, LUA_REGISTRYINDEX, main_module_name.c_str());   //sol.main
    //on_started
    lua_getfield(l, lua_gettop(l) + index + 1, "on_started");   //sol.main(table) on_started(func)
    bool exists = lua_isfunction(l, -1);
    if (exists) {
                                    // ... object ... method
        lua_pushvalue(l, lua_gettop(l) + index + 1);
                                    // ... object ... method object
        if(lua_pcall(l, 1, 0, 0)){
            std::cout << "connot run lua file" << lua_tostring(l, -1) << std::endl;
            lua_pop(l, 1);
        }
    }
    else {
        // Restore the stack.
        lua_pop(l, 1);
                                    // ... object ...
    }
    lua_pop(l, 1);
    //2.入栈操作  
    //lua_pushstring(L, "I am so cool~");   
    //lua_pushnumber(L,20);  

    //3.取值操作  
    //if( lua_isstring(L,1)){             //判断是否可以转为string  
    //    cout<<lua_tostring(L,1)<<endl;  //转为string并返回  
    //}  
    //if( lua_isnumber(L,2)){  
    //    cout<<lua_tonumber(L,2)<<endl;  
    //}  

    //4.关闭state  
    lua_close(l);  
    return 0;  
}