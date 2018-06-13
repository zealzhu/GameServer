/*
Navicat MySQL Data Transfer

Source Server         : aly
Source Server Version : 50722
Source Host           : 120.78.219.164:3306
Source Database       : qmsg

Target Server Type    : MYSQL
Target Server Version : 50722
File Encoding         : 65001

Date: 2018-05-28 12:02:43
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for message
-- ----------------------------
DROP TABLE IF EXISTS `message`;
CREATE TABLE `message` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `qq` bigint(20) NOT NULL,
  `msg` text,
  `time` bigint(20) DEFAULT NULL,
  `group` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
