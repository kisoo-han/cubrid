/*
 * Copyright (C) 2008 Search Solution Corporation. All rights reserved by Search Solution. 
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met: 
 *
 * - Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer. 
 *
 * - Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution. 
 *
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software without 
 *   specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE. 
 *
 */  
  
#include		"odbc_portable.h"
#include		"odbc_env.h"
#include		"sqlext.h"
#include		"odbc_diag_record.h"
#include		"odbc_connection.h"
#include		"odbc_util.h"
#include		"cas_cci.h"
  

/* odbc_environments :
 *		global enviroment handle list head
 */ 
static ODBC_ENV *odbc_environments = NULL;

/************************************************************************
* name:  odbc_alloc_env
* arguments:
*		ODBC_ENV **envptr
* returns/side-effects:
*		RETCODE - odbc api return code
* description:
* NOTE:
************************************************************************/ 
  PUBLIC RETCODE 
{
  
  
  
    
    {
      
      
    
  
  else
    
    {
      
      
      
      
      
	/* Default Attribute value */ 
	env->attr_odbc_version = 0;
      
      
      
      
      
      
    
  



/************************************************************************
* name:  odbc_free_env
* arguments:
*		ODBC_ENV *env 
* returns/side-effects:
*		RETCODE - odbc api return code
* description:
* NOTE:
************************************************************************/ 
  
{
  
  
    
    {
      
	/* HY010 - DM */ 
	odbc_set_diag (env->diag, "HY010", 0, NULL);
      
    
  
    /* remove from list */ 
    for (e = odbc_environments, prev = NULL; e != NULL && e != env; e = e->next)
    
    {
      
    
  
    
    {
      
	
	{
	  
	
      
      else
	
	{
	  
	
    
  
  
  
  
  



/************************************************************************
* name: odbc_set_env_attr
* arguments:
*		ODBC_ENV *env - environment handle
*		attribute - attribute type
*		valueptr - generic value pointer
*		stringlength - SQL_IS_INTEGER(-6) or string length
* returns/side-effects:
*		RETCODE
* description:
*		
* NOTE:
*		diagnostic�� ���ؼ� ���� structure�� ������ ���� �ʾƼ� SQLSTATE�� 
*		�������� ���Ѵ�.  structure�� �ݿ��� �� �� state ���� �����ϵ��� 
*		�Ѵ�.
************************************************************************/ 
  PUBLIC RETCODE 
{
  
    
    {
      
      
    
  
    /* valueptr will be a 32-bit integer value or 
    switch (attribute)
    
    {
    
      
      
      
    
      
      
      
    
      
	
	{
	
	
	  
	  
	
	  
	  
	  
	
      
    
      
	
	{
	
	  
	  
	
	  
	  
	  
	
	  
	  
	  
	
      
    
      
      
      
    
  



/************************************************************************
* name:  odbc_get_env_attr
* arguments:
*		ODBC_ENV *env 
* returns/side-effects:
*		RETCODE - odbc api return code
* description:
* NOTE:
************************************************************************/ 
  PUBLIC 
			      long *string_length_ptr) 
{
  
    
    {
    
      
	
      
	
      
    
      
	
      
	
      
    
      
      
    
  



/************************************************************************
* name: odbc_end_tran
* arguments: 
* returns/side-effects: 
* description: 
* NOTE: 
************************************************************************/ 
  PUBLIC RETCODE 
{
  
  
  
	|| 
													  SQL_HANDLE_DBC
													  
													  &&((ODBC_CONNECTION *) handle)->handle_type != SQL_HANDLE_DBC))
    
    {
      
    
  
    
    {
      
	
	{
	  
	  
	    
	    {
	      
	    
	
    
  
  else
    
    {
      
      
	
	{
	  
	
    
  



/************************************************************************
* name: connection_end_tran
* arguments: 
* returns/side-effects: 
* description: 
* NOTE: 
************************************************************************/ 
  PRIVATE RETCODE 
{
  
  
  
  
  
    
    {
      
    
  
  else
    
    {
      
    
  
    
    {
      
      
	
	{
	  
	  
	
    
  
    // delete all open cursor
    for (stmt = conn->statements; stmt; stmt = stmt->next)
    
    {
      
    
  


