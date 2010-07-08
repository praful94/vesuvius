<?php
/**
* Initialization script for scheduled PFIF import/export services
*
* PHP version 4 and 5
*
* LICENSE: This source file is subject to LGPL license
* that is available through the world-wide-web at the following URI:
* http://www.gnu.org/copyleft/lesser.html
*
* @package    Sahana - http://sahana.sourceforge.net
* @author ccornwell, contractor, NLM-CEB, ccornwell@mail.nih.gov
* @copyright  Lanka Software Foundation - http://www.opensource.lk
*/

include_once "pfifconf.inc.php"; // Declares the configuration structure

// TODO: These methods are Google-specific. They should be generalized as needed.
function add_pfif_service($repository) {
global $pfif_conf;

    $svc = get_service_params($repository->role,
                              $repository->base_url,
                              $repository->params);
    $pfif_conf['map'][$svc['name']] = $repository->id;

    if ($repository->role == 'source') {
        $pfif_conf['services'][$svc['name']]['read_url'] = $svc['read_url'];
        $pfif_conf['services'][$svc['name']]['read_params'] = $svc['read_params'];
        $pfif_conf['services'][$svc['name']]['feed_url'] = $svc['feed_url'];
        $pfif_conf['services'][$svc['name']]['feed_params'] = $svc['feed_params'];
    } else if ($repository->role == 'sink') {
        // TODO: Change this to req_url and req_method
        $pfif_conf['services'][$svc['name']]['post_url'] = $svc['post_url'];
        $pfif_conf['services'][$svc['name']]['req_params'] = $svc['req_params'];
        $pfif_conf['services'][$svc['name']]['auth_key'] = $svc['auth_key'];
    } else {
        // ERROR: invalid role
    }
}

function get_service_params($role, $base, $params) {
    $svc = array();
    $dom_config = DOMDocument::loadXML($params);
    
    // Service Name 
    $el = $dom_config->getElementsByTagName('service');
    if ($el->length > 0) {
        $i = $el->item(0);
        $svc['name'] = $i->getAttribute('name');
    }

    if ($role == 'source') {
        // Set URLs for single-record (read) and multiple record (feed) services
        $r = $el->item(0)->getElementsByTagName('read');
        $ru = $r->item(0)->getElementsByTagName('url');
        $svc['read_url'] = $base.'/'.$ru->item(0)->nodeValue;
        $rp = $r->item(0)->getElementsByTagName('param');
        // TODO: extract and store params
        
        $f = $el->item(0)->getElementsByTagName('feed');
        $fu = $f->item(0)->getElementsByTagName('url');
        $svc['feed_url'] = $base.'/'.$fu->item(0)->nodeValue;
        $fp = $f->item(0)->getElementsByTagName('param');
        // TODO: extract and store params
    } else {
        
    }

    return $svc;
}