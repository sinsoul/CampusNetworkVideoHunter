<?php
if($_GET[action] == install)
{
	require("./ConnectSAEDB.php");
	$ar[0]="SET FOREIGN_KEY_CHECKS=0;";
	$ar[1]="DROP TABLE IF EXISTS `rc_cnvh_update`;";
	$ar[2]="CREATE TABLE `rc_cnvh_update` (`record_id` bigint(20) NOT NULL AUTO_INCREMENT,`record_ip` text,`recode_time` datetime DEFAULT NULL,PRIMARY KEY (`record_id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	$ar[3]="DROP TABLE IF EXISTS `rc_cnvh_blacklist`;";
	$ar[4]="CREATE TABLE `rc_cnvh_blacklist` (`record_id` bigint(20) NOT NULL AUTO_INCREMENT,`record_ip` text,`recode_time` datetime DEFAULT NULL,PRIMARY KEY (`record_id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8;";
	for($i=0;$i<5;$i++)
	{
		echo "Query:".$ar[$i]."</br>";
		$queryresult=mysql_query($ar[$i],$database);
		if($queryresult==false)
		{
			echo '<h5>Query Error:'.mysql_error()."</h5>";
		}
		else
		{
			echo "Success!!</br>";
		}
	}
}

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>统计数据库安装程序</title>
</head>

<body>
<form name=form method=post action=Install_Record.php?action=install>
<table>
	<tr><td colspan=2><input type=submit value=安装></td></tr>
</table>
</form>
</body>
</html>
