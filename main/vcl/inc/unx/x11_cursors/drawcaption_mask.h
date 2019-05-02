/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#define drawcaption_mask_width 32
#define drawcaption_mask_height 32
#define drawcaption_mask_x_hot 8
#define drawcaption_mask_y_hot 8

static const unsigned char drawcaption_mask_bits[] = {
	0x80,0x03,0x00,0x00,
	0x80,0x03,0x00,0x00,
	0x80,0x03,0x00,0x00,
	0x80,0x03,0x00,0x00,
	0x80,0x03,0x00,0x00,
	0x80,0x03,0x00,0x00,
	0x80,0x03,0x00,0x00,
	0xff,0xff,0x01,0x00,
	0xff,0xff,0x01,0x00,
	0xff,0xff,0x01,0x00,
	0x80,0x03,0x00,0x00,
	0x80,0x03,0x00,0x00,
	0x80,0x03,0x00,0x00,
	0x80,0x43,0x00,0x00,
	0x80,0xe3,0xf0,0x3f,
	0x80,0xc3,0xf1,0x3f,
	0x80,0x83,0xff,0x3f,
	0x00,0x00,0x7f,0x38,
	0x00,0x00,0xfe,0x3f,
	0x00,0x00,0xf0,0x3f,
	0x00,0x00,0xf0,0x3f,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00};
