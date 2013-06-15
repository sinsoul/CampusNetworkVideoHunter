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
	$add_record="INSERT INTO `rc_cnvh_blacklist` (`record_ip`, `recode_time`) VALUES ('".$ip."', '".$record_time."')";
	@mysql_query($add_record,$database);
	mysql_close($database);
	echo "202.200.48.29|202.200.48.26|192.168.1.171|210.27.80.201";
?>