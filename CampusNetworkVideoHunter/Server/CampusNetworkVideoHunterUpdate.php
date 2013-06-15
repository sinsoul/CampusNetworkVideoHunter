<?php
	require("./ConnectSAEDB.php");
	if ($HTTP_SERVER_VARS["HTTP_X_FORWARDED_FOR"])
	{
	 $ip = $HTTP_SERVER_VARS["HTTP_X_FORWARDED_FOR"];
	}
	elseif ($HTTP_SERVER_VARS["HTTP_CLIENT_IP"])
	{
	 $ip = $HTTP_SERVER_VARS["HTTP_CLIENT_IP"];
	}
	elseif ($HTTP_SERVER_VARS["REMOTE_ADDR"])
	{
	 $ip = $HTTP_SERVER_VARS["REMOTE_ADDR"];
	}
	elseif (getenv("HTTP_X_FORWARDED_FOR"))
	{
	 $ip = getenv("HTTP_X_FORWARDED_FOR");
	}
	elseif (getenv("HTTP_CLIENT_IP"))
	{
	 $ip = getenv("HTTP_CLIENT_IP");
	}
	elseif (getenv("REMOTE_ADDR"))
	 {
	 $ip = getenv("REMOTE_ADDR");
	}
	else
	{
	 $ip = "Unknown";
	}
	date_default_timezone_set('PRC');
	$now = getdate();
	$record_time=$now[year]."-".$now[mon]."-".$now[mday]." ".$now[hours].":".$now[minutes].":".$now[seconds];
	$add_record="INSERT INTO `rc_cnvh_update` (`record_ip`, `recode_time`) VALUES ('".$ip."', '".$record_time."')";
	@mysql_query($add_record,$database);
	mysql_close($database);
	$parameter = $_GET['parameter'];
	if($parameter=="")
	{
		echo "9";
	}
	else
	{
		$link="Location: http://cwnu-campus-network-video-hunter.googlecode.com/files/".$parameter;
		Header($link);
	}

?>