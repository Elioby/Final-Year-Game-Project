#pragma once

typedef enum asset_type
{
	ASSET_TYPE_UNTRACKED,
	ASSET_TYPE_MESH,
	ASSET_TYPE_IMAGE,
	ASSET_TYPE_SHADER,
	ASSET_TYPE_FONT,
} asset_type_t;

struct asset
{
	char* asset_id;
	asset_type_t asset_type;
};