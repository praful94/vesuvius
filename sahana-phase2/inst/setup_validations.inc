<?php
/**Validation functions of the Sahana Advanced Web Setup
*
* PHP version 4 and 5
*
* LICENSE: This source file is subject to LGPL license
* that is available through the world-wide-web at the following URI:
* http://www.gnu.org/copyleft/lesser.html
*
* @author     Ravith Botejue. <ravithb@yahoo.com>
* @copyright  Lanka Software Foundation - http://www.opensource.lk
* @package    sahana
* @subpackage setup
*/

/**
 * Function to validate an email address.
 *
 * @param String $email Any email address to be checked for validity.
 * 
 * Not Used.
 */
function shn_email_validate($email)
{
	if((eregi("^[_a-z0-9-]+(\.[_a-z0-9-]+)*@[a-z0-9-]+(\.[a-z0-9-]+)*(\.[a-z]{2,3})$", $email))||($email=="")) {
		// if email is a valid one then do nothing ;-)
	}
	else {
		add_error("<b>$email</b> is an invalid email address! please check the 'Administrator Email' again.");
		display_errors();
		shn_setup_form2();
		break;
	}
}// end of shn_email_validate

/**
 * Check whether the general dependencies are satisfied.
 *
 * @return Whether the general dependencies are satisfied.
 */
function shn_setup_validate_general_dependancies()
{
	global $global;

	$continue = true;

	if (!version_compare(phpversion(), '4.1', '>')) $continue = false;
	if (!extension_loaded('gd')) $continue = false;
	if (!extension_loaded('mysql')) $continue = false;
	if (!extension_loaded('gettext')) $continue = false;
	if (!is_writable($global['approot'].'www/tmp')) $continue = false;
	if (!is_writable($global['approot'].'conf')) $continue = false;
	return $continue;
}

/**
 * Validation function for simple database setup page.
 *
 * @return boolean Whether the page contains valid data. true if no errors, false if any.
 */
function shn_setup_validate_simple_dbsetup(){
	global $global;

	$has_errors = false;
	if(isset($_POST['db_type']) && $_POST['db_type']=='mysql'){
		$db = &NewADOConnection('mysql');

		// check host name
		if(isset($_POST['db_host']) && $_POST['db_host']!=''){
			if(preg_match("/:?/",$_POST['db_host'])==false){
				// more than one colon
				add_error(_("The host name has more than one port separator char (:)."));
				$has_errors = true;
			}
			// check for patterns.
			if(stripos($_POST['db_host'],':')>0 ){
				// contains port
				$vals = explode(':',$_POST['db_host']);
				// check if port no is numeric.
				if(is_numeric($vals[1])==false){
					add_error(_("The port number is invalid."));
					$has_errors = true;
				}
			}
		}else{
			add_error(_("The database host:port value cannot be empty."));
			$has_errors = true;
		}

		// check sahana dbname.
		if(isset($_POST['db_name'])==false || $_POST['db_name']==''){
			add_error(_("The database name cannot be empty."));
			$has_errors = true;
		}else{
			if(preg_match('/[!@#$%\^&\\*\/\?`~+=|:;\(\)-]/',$_POST['db_name'])){
				add_error(_("The database name cannot contain special characters."));
				$has_errors = true;
			}
		}

		// check whether user exists,
		// by trying to connect to the default 'mysql' database.,
		$db->Connect($_POST['db_host'],$_POST['root_db_user'],$_POST['root_db_pass'],'mysql');
		if($db->IsConnected()==false){
			add_error(_("Incorrect database username or password."));
			$has_errors = true;
		}else{
			// execute only if connected.
			$res = $db->Execute("SELECT * FROM `user` WHERE `User`='".$_POST['db_user']."'");
			// check whether the user has overidden to keep the user.
			if($db->IsConnected() && (!$res->EOF) && isset($_SESSION['shn_setup_data_keep_ex_user'])){
				// unset the flag. because the user has overridden this.
				unset($_SESSION['setup_system_db_user_exists']);

			}else if($db->IsConnected() && (!$res->EOF)){
				// first time.
				// the database user exists.
				// set flag.
				$_SESSION['setup_system_db_user_exists']=true;
					
				//add_error(_("System database username already exists."));

				$has_errors = true;
			}
			$res = $db->Execute("SELECT * from `db` WHERE `Db`='".$_POST['db_name']."'");
			// check whether the user has overridden operation to drop database.
			if($db->IsConnected() && (!$res->EOF) && isset($_SESSION['shn_setup_data_drop_ex_db'])){
				// unset the flag. The user has overridden this.
				unset($_SESSION['setup_system_db_exists']);

			}else if($db->IsConnected() && (!$res->EOF)){
				$_SESSION['setup_system_db_exists']=true;
				//add_error(_("A database already exists with the given name."));
				$has_errors = true;
			}
			if($db->IsConnected()){
				$db->Close();
			}
		}

		// return the inverse.
		return !$has_errors;

	}else if(isset($_POST['db_type']) && $_POST['db_type']=='pgsql'){
		add_error(_('Unsupported database. Please select a valid database type.'));
		return false;
	}else if(isset($_POST['db_type'])){
		// the set value is not valid.
		add_error(_('Unsupported database. Please select a valid database type.'));
		return false;
	}else{
		return true;
	}
}

/**
 * Validation for the simple configuration page.
 *
 * @return boolean Whether the page contains valid data. true if no errors, false if any.
 */
function shn_setup_validate_simple_config(){
	global $global;
	include_once($global['approot']."inc/lib_security/lib_auth.inc");

	$has_errors = false;

	if(isset($_POST['base_uuid'])==false || strlen($_POST['base_uuid'])==0){
		add_error(_('The unique installation id cannot be empty.'));
		$has_errors = true;
	}

	if(isset($_POST['root_name'])==false || strlen($_POST['root_name'])==0){
		add_error(_('The administrator name cannot be empty.'));
		$has_errors = true;
	}

	if(isset($_POST['admin_name'])==false || strlen($_POST['admin_name'])==0){
		add_error(_('The administrator username cannot be empty.'));
		$has_errors = true;
	}

	if(isset($_POST['root_pwd'])==false || strlen($_POST['root_pwd'])==0){
		add_error(_('The administrator password cannot be empty.'));
		$has_errors = true;
	}else{
		if($_POST['root_pwd']!=$_POST['root_pwd_confirm']){
			add_error(_('The administrator password and confirm password should match'));
			$has_errors = true;
		}
		$ret = shn_check_password_matches_policy($_POST['root_pwd'],$_POST['admin_name'],array('rule_set'=>_shn_setup_filter_post_params($_SESSION)));
		//var_dump($ret);
		if($ret !== true){
			var_dump(shn_get_password_policy_message($ret));
			add_error(_("Administrator Password : ").shn_get_password_policy_message($ret,array('rule_set'=>_shn_setup_filter_post_params($_SESSION))));
			$has_errors = true;
		}
	}

	if(isset($_POST['user_name'])==false || strlen($_POST['user_name'])==0){
		add_error(_('The normal user\'s username cannot be empty.'));
		$has_errors = true;
	}
	if(isset($_POST['user_pwd'])==false || strlen($_POST['user_pwd'])==0){
		add_error(_('The normal user\'s password cannot be empty.'));
		$has_errors = true;
	}else{
		if($_POST['user_pwd']!=$_POST['user_pwd_confirm']){
			add_error(_('The normal user\'s password and confirm password should match.'));
			$has_errors = true;
		}
		$ret = shn_check_password_matches_policy($_POST['user_pwd'],$_POST['user_name'],array('rule_set'=>_shn_setup_filter_post_params($_SESSION)));
		//var_dump($ret);
		if($ret !== true){
			var_dump(shn_get_password_policy_message($ret));
			add_error(_("Normal User Password : ").shn_get_password_policy_message($ret,array('rule_set'=>_shn_setup_filter_post_params($_SESSION))));
			$has_errors = true;
		}
	}

	return !$has_errors;

}

/**
 * Validation function for the advanced setup select modules page.
 *
 * @return unknown Whether the data is valid
 */
function shn_setup_adv_validate_select_modules(){
	return true;
}

/**
 * Validation function for the advanced configuration page.
 *
 * @return boolean Whether the page contains valid data. true if no errors, false if any.
 */
function shn_setup_adv_validate_advanced_config(){
	global $global , $conf;
	$selected_modules = _shn_setup_get_selected_modules();
	$has_errors = false;
	// call module configuration specific validations.
	if(isset($selected_modules)){

		foreach($selected_modules as $sel_mod){
			// include the module specific setup.inc
			$mod_setup_file = $global['approot']."mod/".$sel_mod."/ins/setup.inc";
			if(file_exists($mod_setup_file)){
				include_once($mod_setup_file);
				// the module specific configuration validation callback function.
				$mod_conf_validate_func = "_shn_".$sel_mod."_setup_config_validate";
				if(function_exists($mod_conf_validate_func)){
					// the validation function should return false if not valid data.
					$has_errors |= !($mod_conf_validate_func());
				}
			}
		}
	}
	return !$has_errors;
}

/**
 * Validations for the advanced password policy configuration page.
 *
 * @return boolean Whether the given data is valid. true if no errors, false if any.
 */
function shn_setup_adv_validate_pwd_policy(){
	$has_errors = false;

	if(isset($_POST['pwd_min_chars'])==false || strlen($_POST['pwd_min_chars'])==0){
		add_error(_("The minimum number of characters cannot be empty."));
		$has_errors = true;
	}else{
		if(is_numeric($_POST['pwd_min_chars'])==false){
			add_error(_("The minimum number of characters should be a numeric value"));
			$has_errors = true;
		}else{
			if($_POST['pwd_min_chars']<0){
				add_error(_("The minimum number of characters should be a positive value"));
				$has_errors = true;
			}
		}
	}

	if(isset($_POST['pwd_max_chars'])){
		if(is_numeric($_POST['pwd_max_chars'])==false){
			add_error(_("The maximum number of characters should be a numeric value"));
			$has_errors = true;
		}else{
			if($_POST['pwd_max_chars']<0){
				add_error(_("The maximum number of characters should be a positive value"));
				$has_errors = true;
			}else{
				if($_POST['pwd_max_chars']!=0 && $_POST['pwd_max_chars'] < $_POST['pwd_min_chars']){
					add_error(_("The maximum number of characters cannot be less than the minimum number of characters."));
					$has_errors = true;
				}
			}
		}
	}

	return !$has_errors;
}


?>