<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
 */
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class FTPRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"setSettings" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"port":{"type":"integer","minimum":1,"maximum":65535},
					"maxclients":{"type":"integer","minimum":0},
					"maxconnectionsperhost":{"type":"integer","minimum":0},
					"maxloginattempts":{"type":"integer","minimum":0},
					"timeoutidle":{"type":"integer","minimum":0},
					"displaylogin":{"type":"string"},
					"allowforeignaddress":{"type":"boolean"},
					"allowrestart":{"type":"boolean"},
					"identlookups":{"type":"boolean"},
					"usereversedns":{"type":"boolean"},
					"rootlogin":{"type":"boolean"},
					"masqueradeaddress":{"type":"string"},
					"usepassiveports":{"type":"boolean"},
					"minpassiveports":{"type":"integer","minimum":1025,'.
					  '"maximum":65535},
					"maxpassiveports":{"type":"integer","minimum":1025,'.
					  '"maximum":65535},
					"limittransferrate":{"type":"boolean"},
					"maxuptransferrate":{"type":"integer","minimum":0},
					"maxdowntransferrate":{"type":"integer","minimum":0},
					"anonymous":{"type":"boolean"},
					"requirevalidshell":{"type":"boolean"},
					"extraoptions":{"type":"string"}
				}
			}'),
			"getShareList" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}' // sortDir
			),
			"getShare" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"setShare" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"sharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
					"mntentref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"comment":{"type":"string"}
				}
			}'),
			"deleteShare" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"getModBanRuleList" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}' // sortDir
			),
			"getModBanRule" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"setModBanRule" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"event":{"type":"string","enum":["AnonRejectPasswords",'.
					  '"ClientConnectRate","MaxClientsPerClass",'.
					  '"MaxClientsPerHost","MaxClientsPerUser",'.
					  '"MaxConnectionsPerHost","MaxHostsPerUser",'.
					  '"MaxLoginAttempts","TimeoutIdle","TimeoutNoTransfer"]},
					"occurrence":{"type":"integer","minimum":1},
					"timeinterval":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[0-9]{2}:[0-9]{2}:[0-9]{2}$\/"},
					"expire":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[0-9]{2}:[0-9]{2}:[0-9]{2}$\/"}
				}
			}'),
			"deleteModBanRule" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"setModTLSSettings" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"required":{"type":"boolean"},
					"sslcertificateref":{'.
					  $GLOBALS['OMV_JSONSCHEMA_UUID_NONE'].'}
				}
			}')
		);
	}

	public function getSettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/ftp";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		unset($object['shares']);
		unset($object['modules']);
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['port'] = intval($object['port']);
		$object['maxclients'] = intval($object['maxclients']);
		$object['maxconnectionsperhost'] = intval(
		  $object['maxconnectionsperhost']);
		$object['maxloginattempts'] = intval($object['maxloginattempts']);
		$object['timeoutidle'] = intval($object['timeoutidle']);
		$object['allowforeignaddress'] = boolval(
		  $object['allowforeignaddress']);
		$object['allowrestart'] = boolval($object['allowrestart']);
		$object['identlookups'] = boolval($object['identlookups']);
		$object['usereversedns'] = boolval($object['usereversedns']);
		$object['rootlogin'] = boolval($object['rootlogin']);
		$object['usepassiveports'] = boolval($object['usepassiveports']);
		$object['minpassiveports'] = intval($object['minpassiveports']);
		$object['maxpassiveports'] = intval($object['maxpassiveports']);
		$object['limittransferrate'] = boolval($object['limittransferrate']);
		$object['maxuptransferrate'] = intval($object['maxuptransferrate']);
		$object['maxdowntransferrate'] = intval($object['maxdowntransferrate']);
		$object['anonymous'] = boolval($object['anonymous']);
		$object['requirevalidshell'] = boolval($object['requirevalidshell']);
		return $object;
	}

	public function setSettings($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get existing configuration object
		$xpath = "//services/ftp";
		$oldObject = $xmlConfig->get($xpath);
		if (is_null($oldObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update configuration
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"port" => $data['port'],
			"maxclients" => $data['maxclients'],
			"maxconnectionsperhost" => $data['maxconnectionsperhost'],
			"maxloginattempts" => $data['maxloginattempts'],
			"timeoutidle" => $data['timeoutidle'],
			"displaylogin" => $data['displaylogin'],
			"allowforeignaddress" => array_boolval($data,
			  'allowforeignaddress'),
			"allowrestart" => array_boolval($data, 'allowrestart'),
			"identlookups" => array_boolval($data, 'identlookups'),
			"usereversedns" => array_boolval($data, 'usereversedns'),
			"rootlogin" => array_boolval($data, 'rootlogin'),
			"masqueradeaddress" => $data['masqueradeaddress'],
			"usepassiveports" => array_boolval($data, 'usepassiveports'),
			"minpassiveports" => $data['minpassiveports'],
			"maxpassiveports" => $data['maxpassiveports'],
			"limittransferrate" => array_boolval($data, 'limittransferrate'),
			"maxuptransferrate" => $data['maxuptransferrate'],
			"maxdowntransferrate" => $data['maxdowntransferrate'],
			"anonymous" => array_boolval($data, 'anonymous'),
			"requirevalidshell" => array_boolval($data, 'requirevalidshell'),
			"extraoptions" => $data['extraoptions'],
			"shares" => isset($oldObject['shares']) ?
			  $oldObject['shares'] : array(),
			"modules" => isset($oldObject['modules']) ?
			  $oldObject['modules'] : array()
		);
		if (FALSE === $xmlConfig->replace("//services/ftp", $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp", $object);
	}

	/**
	 * Get list of share config objects.
	 */
	public function getShareList($start, $count, $sortField, $sortDir) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath = "//services/ftp/shares/share";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations
		foreach ($objects as $objectk => &$objectv) {
			$objectv['sharedfoldername'] = gettext("n/a");
			// Get the shared folder configuration object
			$sfobject = $xmlConfig->get("//system/shares/sharedfolder[".
			  "uuid='{$objectv['sharedfolderref']}']");
			if (!is_null($sfobject) && !empty($sfobject)) {
				$objectv['sharedfoldername'] = $sfobject['name'];
			}
		}
		// Filter result
		return $this->applyFilter($objects, $start, $count, $sortField,
		  $sortDir);
	}

	/**
	 * Get a share config object.
	 */
	public function getShare($uuid) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath = sprintf("//services/ftp/shares/share[uuid='%s']", $uuid);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return $object;
	}

	/**
	 * Set a share config object.
	 */
	public function setShare($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Prepare the configuration object.
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"sharedfolderref" => $data['sharedfolderref'],
			"mntentref" => $data['mntentref'],
			"comment" => $data['comment']
		);
		// Set the configuration object.
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness. A shared folder can only be configured once
			// for FTP.
			if (TRUE === $xmlConfig->exists("//services/ftp/shares/share[" .
			  "sharedfolderref='{$data['sharedfolderref']}']")) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				gettext("The shared folder is already used by another share"));
			}
			// Get the shared folder object.
			$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
			  $data['sharedfolderref']);
			$sfobject = $xmlConfig->get($xpath);
			if (is_null($sfobject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Get the mount point configuration object.
			$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
			  $sfobject['mntentref']);
			$meobject = $xmlConfig->get($xpath);
			if (is_null($meobject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Prepare fstab mount point entry configuration object to bind
			// the shared folder directory into the FTP users home directory.
			$meObject = array(
				"uuid" => OMVUtil::uuid(),
				"fsname" => sprintf("%s/%s", $meobject['dir'],
				  $sfobject['reldirpath']),
				"dir" => sprintf("/home/ftp/%s", $sfobject['name']),
				"type" => "none",
				"opts" => "bind",
				"freq" => 0,
				"passno" => 0
			);
			// Modify the configuration object.
			$object['mntentref'] = $meObject['uuid'];
			// Append the configuration object.
			$success = $xmlConfig->set("//services/ftp/shares",
			  array("share" => $object));
			// Append assosiated fstab mount point entry configuration object.
			if (FALSE === $xmlConfig->set("//system/fstab", array(
			  "mntent" => $meObject))) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
			}
		} else {
			// Update the existing configuration object.
			$success = $xmlConfig->replace("//services/ftp/shares/share[" .
			  "uuid='{$data['uuid']}']", $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			$dispatcher->notify(OMV_NOTIFY_CREATE,
			  "org.openmediavault.system.fstab.mntent", $meObject);
		}
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp.shares.share", $object);
	}

	/**
	 * Delete a share config object.
	 */
	public function deleteShare($uuid) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get the object
		$xpath = sprintf("//services/ftp/shares/share[uuid='%s']", $uuid);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.services.ftp.shares.share", $object);
		// Delete the configuration object.
		if (FALSE === ($object = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
		// Save the configuration.
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.ftp.shares.share", $object);
		// Delete the associated fstab mount point entry.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $object['mntentref']);
		if (FALSE === ($meObject = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
		// Save the configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.fstab.mntent", $meObject);
	}

	/**
	 * Get list of mod_ban rule config objects.
	 */
	public function getModBanRuleList($start, $count, $sortField, $sortDir) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get list of configuration objects
		$xpath = "//services/ftp/modules/mod_ban/rule";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		foreach ($objects as $objectk => &$objectv) {
			$objectv['occurrence'] = intval($objectv['occurrence']);
		}
		// Filter result
		return $this->applyFilter($objects, $start, $count, $sortField,
		  $sortDir);
	}

	/**
	 * Get a mod_ban rule config object.
	 */
	public function getModBanRule($uuid) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath = "//services/ftp/modules/mod_ban/rule[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['occurrence'] = intval($object['occurrence']);
		return $object;
	}

	/**
	 * Set a mod_ban rule config object.
	 */
	public function setModBanRule($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Prepare configuration data
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"event" => $data['event'],
			"occurrence" => $data['occurrence'],
			"timeinterval" => $data['timeinterval'],
			"expire" => $data['expire']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration
			$success = $xmlConfig->set("//services/ftp/modules/mod_ban",
			  array("rule" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace("//services/ftp/modules/mod_ban" .
			  "/rule[uuid='{$data['uuid']}']", $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp.modules.mod_ban.rule", $object);
	}

	/**
	 * Delete a mod_ban rule config object.
	 */
	public function deleteModBanRule($uuid) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Delete configuration object
		$xpath = "//services/ftp/modules/mod_ban/rule[uuid='{$uuid}']";
		if (FALSE === ($object = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.ftp.modules.mod_ban.rule", $object);
	}

	public function getModTLSSettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/ftp/modules/mod_tls";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['required'] = boolval($object['required']);
		return $object;
	}

	public function setModTLSSettings($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Set configuration object
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"required" => array_boolval($data, 'required'),
			"sslcertificateref" => $data['sslcertificateref']
		);
		$xpath = "//services/ftp/modules/mod_tls";
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp.modules.mod_tls", $object);
	}

	/**
	 * Get statistics, e.g. who's online.
	 */
	public function getStats() {
		global $xmlConfig;
		// Is service enabled?
		$xpath = "//services/ftp";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if (!array_boolval($object, 'enable')) {
			$stats = gettext("Service disabled");
		} else {
			// Get users currently logged in
			$stats = sprintf("%s\n", gettext("Currently logged in users:"));
			$cmd = "sudo ftpwho -v 2>&1";
			OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			array_shift($output);
			$stats .= implode("\n", $output);

			// Get ban list if module is enabled
			$xpath = "//services/ftp/modules/mod_ban/rule";
			if (TRUE === $xmlConfig->exists($xpath)) {
				$stats .= sprintf("\n\n%s\n", gettext("Ban statistics:"));
				unset($output);
				$cmd = "sudo ftpdctl -s /var/run/proftpd/proftpd.sock ban info -v -e 2>&1";
				OMVUtil::exec($cmd, $output, $result);
				if ($result !== 0) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
				foreach ($output as &$outputv) {
					$outputv = preg_replace("/^ftpdctl:/", "", $outputv);
				}
				$stats .= implode("\n", $output);
			}
		}
		return $stats;
	}
}
?>
