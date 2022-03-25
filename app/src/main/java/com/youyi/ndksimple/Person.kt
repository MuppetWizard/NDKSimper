package com.youyi.ndksimple

/**
 * des：
 * @author: Muppet
 * @date: 2022/3/23
 */
data class Person(
    var name: String,
    var age: Int,
    var gen: String
){
    constructor():this("",0,"")
}
